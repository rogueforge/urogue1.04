/*
 * Last Modified: 07/27/86
 */
/*
 * save and restore routines
 *
 */

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "rogue.h"
#include "mach_dep.h"

typedef struct stat STAT;

extern char *sys_errlist[], version[];
extern int errno;

char *sbrk(), _putchar();

STAT sbuf;

int sigdie = -1;

save_game()
{
    register int savefd;
    register int c;
    char buf[2*LINELEN];

    /*
     * get file name
     */
    mpos = 0;
    if (file_name[0] != '\0') {
	msg("Save file (%s)? ", file_name);
	do {
	    c = getchar() & 0177;
	    if (c == ESCAPE) return(0);
	} while (c != 'n' && c != 'N' && c != 'y' && c != 'Y');
	mpos = 0;
	if (c == 'y' || c == 'Y')
	    goto gotfile;
    }

    do {
	msg("File name: ");
	mpos = 0;
	buf[0] = '\0';
	if (get_str(buf, cw) == QUIT) {
	    msg("");
	    return FALSE;
	}
	strcpy(file_name, buf);
gotfile:
	wclear(hw);
	wmove(hw, LINES-1, 0);
	wrefresh(hw);
	if ((savefd = creat(file_name, 0400)) == -1)
	    msg(sys_errlist[errno]);	/* fake perror() */
    } while (savefd == -1);

    /*
     * Fix keypad?
     */
    if(keypadmode) {
	keypadoff();
    }

    /*
     * write out encrpyted file (after a stat)
     * The fwrite is to force allocation of the buffer before the write
     */
    save_file(savefd);
    return TRUE;
}

/*
 * automatically save a file.  This is used if a HUP signal is
 * recieved
 */
auto_save(sig)
int sig;
{
    register int savefd;

    /*
     * Code added by Bruce Dautrich on 4/11/84 to
     * increase probability of saving game if two 
     * signals are sent to game. Requires define
     * in rogue.h of a variable called sigdie
     */

    if (sigdie == -1) {
	/*This is first signal*/
	sigdie=sig;
	signal(sig,SIG_IGN);
	goto got1sig;
    } else {
	/*This is second signal*/
	signal(sig,SIG_IGN);
    }

got1sig:
    if (file_name[0] != '\0' && (savefd = creat(file_name, 0400)) > -1)
	save_file(savefd);
#if defined(BSD4)
    (void) TtySetOriginalChars();
#endif
    endwin();
    exit(1);
}

save_file(savefd)
register int savefd;
{
    wmove(cw, LINES-1, 0);
    wrefresh(cw);
    fstat(savefd, &sbuf);
    write(savefd, "junk", 5);
    lseek(savefd, 0L, 0);
    encwrite(version, sbrk(0) - version, savefd);
    close(savefd);
}

restore(file, envp)
register char *file;
char **envp;
{
    register int infd;
    extern char **environ;
    register char *sp;
    char buf[2*LINELEN];
    STAT sbuf2;

    makesure();

    /*
     * Reset the effective uid & gid to the real ones.
     */
    seteuid(getuid());
    setegid(getgid());

    if (strcmp(file, "-r") == 0)
	file = file_name;
    if ((infd = open(file, 0)) < 0) {
	perror(file);
	return FALSE;
    }

    fflush(stdout);
    encread(buf, strlen(version) + 1, infd);
    if (strcmp(buf, version) != 0) {
	printf("Save Game Version: %s\nReal Game Version: %s\n", buf, version);
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }

    fstat(infd, &sbuf2);
    fflush(stdout);
    brk(version + sbuf2.st_size);
    lseek(infd, 0L, 0);
    encread(version, (int) sbuf2.st_size, infd);

    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    if (!wizard) {
	if (sbuf2.st_ino != sbuf.st_ino || sbuf2.st_dev != sbuf.st_dev) {
	    printf("Sorry, saved game is not in the same file.\n");
	    return FALSE;
	}
	else if (sbuf2.st_ctime - sbuf.st_ctime > SAVETIME) {
	    printf("(%d): Sorry, file has been touched.\n",
		sbuf2.st_ctime - sbuf.st_ctime);
	    return FALSE;
	}
    }
    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    /*
     * defeat multiple restarting from the same place
     */
    if (!wizard) {
	if (sbuf2.st_nlink != 1) {
	    printf("Cannot restore from a linked file\n");
	    return FALSE;
	}
	else if (unlink(file) < 0) {
	    printf("Cannot unlink file\n");
	    return FALSE;
	}
    }

#if defined(BSD4)
    (void) TtyInit();
    (void) TtyGetGameChars();
    (void) TtyMapGameChars();
#endif

    environ = envp;
    if ((sp = getenv(OPTIONS)) != NULL)
	parse_opts(sp);
#if defined(USGV5)
    if (isatty(2)) {
#else
    if (!My_term && isatty(2)) {
#endif
	_tty_ch = 2;
#if !defined(USGV5)
	gettmode();
#endif
	if ((sp = getenv("TERM")) == NULL)
	    sp = Def_term;
    }
    else
	sp = Def_term;
#if defined(attron)
    savetty();
    restartterm(sp, 1, 0);
    resetty();
#else
    setterm(sp);
#endif attron
#if !defined(USGV5)
    _puts(TI);
    _puts(VS);
#endif

    /*
     * Should we turn keypad mode on?
     */
    if(keypadmode) {
	keypadon();
    }

    strcpy(file_name, file);
    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
#if defined(attron)
    cbreak();
    noecho();
    nonl();
#else
#if defined(USGV5)
    /*
     * Sys V curses weirdness
     */
    noecho();
    nonl();
#endif
#endif attron
    srand(getpid());
    playit();
    /*NOTREACHED*/
}

/*****************************************************************
 *
 *           encwrite, encread: encoded read/write routines
 *
 * The encwrite/encread routines implement a synchronous stream Vernam
 * cipher using a linear congruential random number generator to
 * simulate a one-time pad.  The random seed is encoded and stored in
 * the file using data diffusion (putword,getword).
 *
 * Usage:
 *	encwrite (start, size, outfd);
 *	unsigned char *start; int size; int outfd;
 *
 *	encread (start, size, infd);
 *	unsigned char *start; int size; int infd;
 *
 * HISTORY
 *
 * 07-May-86  Michael Laman (laman) at NCR Rancho Bernardo
 *	Modified for UltraRogue.  Removed all uses of standard I/O from
 *	data encryption routines.  This is most portable solution around
 *	a bug of limiting how many times a game may be saved.
 *
 * 03-Mar-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Modified for UltraRogue.  Allowed multiple encwrites to a file by
 *	removing lseek calls and computing checksum while reading.
 *
 * 20-Dec-82  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created as a replacement for the encwrite/encread in
 *	Rogue 5.2, which are very easily broken.
 *
 *****************************************************************/

/* Constants for key generation */
# define OFFSET		667818
# define MODULUS	894871
# define MULT		2399
# define ENCHAR		((unsigned char) ((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10))

/* Constants for checksumming */
# define INITCK		1232531
# define CKMOD		6506347

struct wb { int w1, w2, w3, w4; };

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/

encwrite (start, size, outfd)
register unsigned char *start;
int size;
int outfd;
{
  register unsigned char *buf_p, *end_p;
  register int cksum = INITCK, seed, ch;
  register int cnt;
  char buffer[512 * 20];

  srand (time ((long *) 0) + getpid () + getuid ());	/* Build a random seed */
  seed = (rand () & 0x7fff);
  
  putword (seed, outfd);			/* Write the random seed */
  putword (size, outfd);			/* Write the file length */

# ifdef DEBUG
  fprintf (stderr, "Encwrite: size %ld, seed %ld.\n", size, seed);
# endif

  /* Now checksum, encrypt, and write out the buffer. */
  while (size)
  {
    buf_p = (unsigned char *) buffer;
    cnt = min(size, sizeof buffer);
    end_p = (unsigned char *) &buffer[cnt];
    while(buf_p < end_p) {
	cksum = ((cksum << 8) + (ch = *start++)) % CKMOD;
	*buf_p++ = ch ^ ENCHAR;
    }
    if(write(outfd, buffer, cnt) != cnt)
	return 0;
    size -= cnt;
  }

  putword (cksum, outfd);			/* Write out the checksum */

# ifdef DEBUG
  fprintf (stderr, "Checksum is %ld.\n", cksum);
# endif
  return 1;
}

/*****************************************************************
 * Encread: read a block of encrypted text from a file descriptor
 *****************************************************************/

encread (start, size, infd)
register unsigned char *start;
int size;
int infd;
{ register int cksum = INITCK, seed, length, cnt, ch;
  int stored, getword();

  seed = getword (infd); 
  stored = getword (infd);  

# ifdef DEBUG
  fprintf (stderr, "Encread: size %ld, seed %ld, stored %ld.\n", 
	   size, seed, stored);
# endif
  
  if ((length = read (infd, start, (int) min (size, stored))) > 0)
  { for (cnt = length; cnt--; ) 
    { ch = (unsigned char) (*start++ ^= ENCHAR);
      cksum = ((cksum << 8) + ch) % CKMOD;
    }
  
    if ((length == stored) && (getword (infd) != cksum))
    {
# ifdef DEBUG
      fprintf (stderr, "Computed checksum %ld is wrong.\n", cksum);
# else
      fprintf (stderr, "Sorry, file has been touched.\n");
# endif
      while (length--) *--start = '\0';		/* Zero the buffer */
    }
  }
  
  return (length);
}

/*****************************************************************
 * putword: Write out an encoded int word
 *****************************************************************/

putword (word, file)
int word;
int file;
{ struct wb w;

  w.w1 = rand ();  
  w.w2 = rand ();  
  w.w3 = rand ();  
  w.w4 = w.w1 ^ w.w2 ^ w.w3 ^ word;
  
  return write (file, (char *) &w, sizeof w);
}

/*****************************************************************
 * getword: Read in an encoded int word
 *****************************************************************/

getword (fd)
int fd;
{ struct wb w;
  
  if (read (fd, (char *) &w, sizeof w) == sizeof w)
    return (w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
  else
    return (0);
}
