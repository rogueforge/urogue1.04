/*
 * Last Modified: 08/09/86
 */
 /*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include "rogue.h"
#include "mach_dep.h"
#include "score.h"
#if defined(USGV5) || defined(RTU2)
#include <time.h>
#else
#include <sys/time.h>
#endif

static int num_checks;		/* times we've gone over in checkout() */
static double avec[3];		/* load average vector */

int fd_score = -1;		/* file descriptor the score file */

main(argc, argv, envp)
char **argv;
char **envp;
{
    register int x;
    register char *env;
    register struct passwd *pw;
    register struct linked_list *item;
    register struct object *obj;
    struct passwd *getpwuid();
    char *getpass(), *crypt(), *strrchr();
    char *pagerbase;
    FILE *filep;
    int quit(), lowtime, wpt, i, j, hpadd, dmadd;
    void areuok(), init_exp();
    bool alldone,predef;
    long now;
	int rflag;

    for (x = 1; x < argc; x++) {
    	if (argv[x][0] != '-')
    	    break;
    	switch (argv[x][1]) {
    	    when 's':
		prscore = TRUE;
    	    when 'k':
		keypadmode = TRUE;
    	    when 'n':
		prnews = TRUE;
    	    when 'v':
		prversion = TRUE;
	    when 'r':
		rflag = TRUE;
	    otherwise:
		fprintf(stderr, "%s: Unknown option '%c'.\n",
		    argv[0], argv[x][1]);
		exit(1);
	}
    }
    if(!rflag) {
    	argc -= (x - 1);
    	argv += (x - 1);
    }

    areuok(PERMOK);				/* read author file */
    (void) signal(SIGQUIT, SIG_IGN); 		/* ignore quit for now */

    /*
     * get home and options from environment
     */
    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else if ((pw = getpwuid(getuid())) != NULL)
	strcpy(home, pw->pw_dir);
    else
	home[0] = '\0';
    strcat(home, "/");

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, ROGUE_SAVE);

    /* Get default score file */
    strcpy(score_file, SCOREDIR);

    /*
     * Grab a file descriptor to the score file before the
     * effective uid and gid are reset to the real ones.
     */
    fd_score = open(score_file, 2);

    if ((env = getenv(OPTIONS)) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	if ((pw = getpwuid(getuid())) == NULL) {
	    printf("Say, who the hell are you?\n");
	    printf("You can't run without a name.\n");
	    sleep(4);
	    exit(1);
	}
	else
	    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
    if (env == NULL || fruit[0] == '\0') {
	static char *funfruit[] = {
		"candleberry", "caprifig", "dewberry", "elderberry",
		"gooseberry", "guanabana", "hagberry", "ilama", "imbu",
		"jaboticaba", "jujube", "litchi", "mombin", "pitanga",
		"prickly pear", "rambutan", "sapodilla", "soursop",
		"sweetsop", "whortleberry"
	};

	srand48((long) (getpid()+255));
	strcpy(fruit, funfruit[rnd(sizeof(funfruit)/sizeof(funfruit[0]))]);
     }

     /* put a copy of fruit in the right place */
     strcpy(fd_data[1].mi_name, fruit);

    /*
     * print scores
     */
    if (prscore) {
	waswizard = TRUE;
#if defined(BSD4)
	(void) TtyInit();		/* Set up for the tty magic */
	(void) TtyMapGameChars();
#endif
	score(0L, 0, SCOREIT, 0);
	exit(0);
    }

    /*
     * print news 
     */
    if (prnews) {
	pagerbase = strrchr(PAGER, '/')+1;
	(void) execl(PAGER, pagerbase, NEWS, 0);
	exit(0);
    }

    /*
     * check for version option
     */
    if (prversion) {
	printf("UltraRogue Version %s.\n", release);
	exit(0);
    }

    /*
     * check for NOPLAY file 
     */
    if ((filep = fopen(NOPLAY, "r")) != NULL) {
	register int ch;
	while((ch = getc(filep)) != EOF)
		putchar(ch);
	putchar('\n');
	exit(0);
    }

    /*
     * Check to see if he is a wizard
     */
    makesure();
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), PASSWDSEED)) == 0) {
		if (canwizard) {
		    signal(SIGQUIT, SIG_DFL);
		    wizard = TRUE;
		    argv++;
		    argc--;
		}
		else
			printf("Well that's odd...\n");
	}

    if( !author() && !wizard && !holiday()) {
	static char *niceties[] = { "juicy", "fresh", "nice" ,"sweet" };

	srand48((long) (getpid()+rnd(127)));
	printf("Sorry, %s, but you can't play during working hours.\n", whoami);
	printf("Meanwhile, why not enjoy a %s %s?\n",niceties[rnd(4)], fruit);
	sleep(4);
	exit(1);
    }
    if (too_much() && !wizard) {
	if(!author()) {
	    static char *niceties[] = { "juicy", "fresh", "nice" ,"sweet" };

	    srand48((long) (getpid()+rnd(127)));
	    printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	    printf("Try again later.  Meanwhile, why not enjoy a %s %s?\n",
	        niceties[rnd(4)], fruit);
	    sleep(4);
	    exit(1);
	}
	else {
		printf("The system load is rather heavy, %s...\n",whoami);
		printf("However I'll let you play anyway!\n");
		printf("Please note that there are %d users!\n",ucount());
		sleep(2);
	}
    }

    if ((filep = fopen(MOTD, "r")) != NULL) {
	register int ch;
	while((ch = getc(filep)) != EOF)
		putchar(ch);
	putchar('\n');
	printf(retstr);
	gets(prbuf);
	fclose(filep);
    }

    /*
     * Should we put the terminal's keypad into application's
     * mode for the user?
     */
    if(keypadmode) {
	keypadon();
    }

    if (argc == 2 && argv[1][0] != '\0') {
	if (!restore(argv[1], envp)) { /* Note: restore returns on error only */
	    exit(1);
	}
    }
    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    seed = dnum;
    srand48((long) seed);

    init_things();			/* Set up probabilities of things */
    init_fd();				/* Set up food probabilities */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
#if defined(BSD4)
    (void) TtyInit();			/* Set up for the tty magic */
#endif
    initscr();				/* Start up cursor package */
#if defined(attron)
    typeahead(-1);	/* Turn off typeahead checking */
#endif attron
    init_names();			/* Set up names of scrolls */
    setup();

    /*
     * Reset the effective uid & gid to the real ones.
     */
    seteuid(getuid());
    setegid(getgid());

    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    waswizard = wizard;			/* set wizard flags */

    init_player();			/* look up things and outfit pack */

    resurrect = pstats.s_const;
    init_exp();				/* set first experience level change */
    init_flags();			/* set initial flags */
    wclear(hw);
    wrefresh(hw);
    new_level(POSTLEV);			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, (int) &player, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */

endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    wrefresh(stdscr);
    endwin();
    printf("\n");	/* So the cursor doesn't stop at the end of the line */
#if defined(BSD4)
    (void) TtySetOriginalChars();
#endif
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

rnd(range)
register int range;
{
	return(range <= 0 ? 0 : (lrand48() & 0x7fffffff) % range);
}

/*
 * roll:
 *	roll a number of dice
 */

roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}

setup()
{
    void areuok();
    int  checkout();

#if defined(BSD4)
    int  tstop();
    signal(SIGTSTP, tstop);
#endif

    signal(SIGHUP, auto_save);
    signal(SIGINT, quit);

#if !defined(USGV5)
    if (!author()) {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
	num_checks = 0;
    }
#endif
#if defined(attron)
    cbreak();
#else
    crmode();				/* Cbreak mode */
#endif attron
    noecho();				/* Echo off */
    nonl();
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

playit()
{
    register char *opts;

    /*
     * set up defaults for slow terminals
     */

#if defined(attron)
	if(baudrate() < B1200)
#else
#if defined(USGV5) || defined(USGV3)
	if((_tty.c_cflag & CBAUD) < B1200)
#endif
#if defined(BSD4)
	if(_tty.sg_ospeed < B1200)
#endif
#if defined(RTU2)
	if(TRUE == FALSE)  /* dummy if since RTU screws up BSD simulation */
#endif
#endif attron
    {
	terse = TRUE;
	jump = TRUE;
	inv_type = INV_SLOW;
    } 

    char_type = player.t_ctype;

    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();			/* Command execution */
    endit();
}

/*
 * see if the system is being used too much for this game
 */
too_much()
{
#if !defined(USGV5)
	loadav(avec);
	return (avec[2] > (MAXLOAD / 10.0)) || (ucount() > MAXUSERS);
#else
	return (0);
#endif
}

/*
 * author:
 *	See if a user is an author of the program
 *	or a user with the author's permission.
 */
author()
{
	int	id;
	int	i;

	id = getuid();

	for(i=0;i<MAXAUTH;i++)
		if(id == auth_or[i])
			return TRUE;
	return FALSE;
}

makesure()
{

	int	fd,i;

	canwizard = FALSE;
	if ((fd = open(PASSCTL, 0)) < 0) return (FALSE);

	for ( i=0; i<20;i++)
		rpass.rp_pass[i] = 0;
	for ( i=0; i<2;i++)
		rpass.rp_pkey[i] = 0;

	encread(rpass.rp_pkey, (int) sizeof(rpass.rp_pkey), fd);
	encread(rpass.rp_pass, (int) sizeof(rpass.rp_pass), fd);

	close(fd);

	if((*(rpass.rp_pkey) != 0) && (*(rpass.rp_pass) != 0))
	    canwizard = TRUE;
	return(FALSE);
}
void
areuok(file)
char	*
file;
{
	register int	fd;
	char	idbuf[20];
	int	i;

	if((fd = open(file,0)) < 0)
		return ;

	encread(auth_or, (int) sizeof(auth_or), fd);

	close(fd);
}

#if !defined(USGV5)
checkout()
{
	static char *msgs[] = {
	"The system is too loaded for games. Please leave in %d minutes.",
	"Please save your game.  You have %d minutes.",
	"This is your last chance. You had better leave in %d minutes.",
	};
	int checktime;

	signal(SIGALRM, checkout);
	if (too_much() || !holiday()) {
	    if (num_checks >= 3) {
		register FILE *savef;
		FILE *fopen();

		clear();
		move(LINES-2, 0);
		printw("You didn't listen, so now you are DEAD!!\n");
		wrefresh(stdscr);
		endwin();
		printf("\n");	/* now do a save if possible */
		if (file_name[0] != '\0' 
			&& (savef = fopen(file_name, "w")) != NULL)
		    save_file(savef);
#if defined(BSD4)
		(void) TtySetOriginalChars();
#endif
		exit(1);
	    }
	    checktime = WARNTIME;
	    chmsg(msgs[num_checks++], checktime);
	    alarm(checktime * 60);
	}
	else {
	    if (num_checks) {
		chmsg("The load has dropped. You have a reprieve.");
		num_checks = 0;
	    }
	    alarm(CHECKTIME * 60);
	}
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
/*VARARGS1*/
chmsg(fmt, arg)
char *fmt;
int arg;
{
	if (in_shell) {
		printf(fmt, arg);
		putchar('\n');
		fflush(stdout);
	}
	else
		msg(fmt, arg);
}

/*
 * loadav - find current load average
 */
loadav(avg)
reg double *avg;
{
	FILE * pp;
	FILE * popen ();
	extern char LOADAV_PROG[];
	extern char LOADAV_SCAN[];

	if( (pp = popen (LOADAV_PROG, "r")) == (FILE*) 0) {
		avg[0] = 0;
		avg[1] = 0;
		avg[2] = 0;
		return;
	}
	fscanf(pp, LOADAV_SCAN, &avg[0], &avg[1], &avg[2]);
	pclose (pp);
	return;
}
#endif

/*
 * ucount:
 *	Count the number of people on the system
 */
#if defined(USGV5) || defined(RTU3)
/*Must be a 5.0 UNIX system */
#include <sys/types.h>
#include <utmp.h>
struct utmp buf;
ucount()
{
	reg struct utmp *up;
	reg FILE *utmp;
	reg int count;

	if ((utmp = fopen(UTMP, "r")) == NULL)
	    return 0;

	up = &buf;
	count = 0;
	while (fread((char *) up, 1, sizeof (*up), utmp) > 0)
		if (buf.ut_type == USER_PROCESS)
			count++;
	fclose(utmp);
	return count;
}
#endif

#if !defined (RTU3) && defined(BSD4) || defined(USGV3) || defined(RTU2)
#include <utmp.h>

struct utmp buf;

ucount()
{
    register struct utmp *up;
    register FILE *utmp;
    register int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread((char *) up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif

/*
 * holiday:
 *	Returns TRUE when it is a good time to play rogue
 */
holiday()
{
	long now;
	struct tm *localtime();
	reg struct tm *ntime;

	if (!HOLIDAY)
		return TRUE;

	time(&now);			/* get the current time */
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return TRUE;		/* OK on Sat & Sun */
	if(ntime->tm_hour < 9 || ntime->tm_hour >= 17)
		return TRUE;		/* OK before 9AM & after 5PM */
	return FALSE;			/* All other times are bad */
}
