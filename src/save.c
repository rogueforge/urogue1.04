/*
 * Last Modified: Dec 30, 1990
 */
/*
 * save and restore routines
 *
 */

#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rogue.h"

typedef struct stat STAT;

extern char version[], *strerror();
#include <errno.h>

extern char __executable_start[], _end[];
extern char rg_bss[], *const rg_edata, *const rg_ebss;
extern int playit(), rg_restored();
long save_size();

long save_stamp[2];			/* tell one build from another */
char save_win[3][MAXLINES][MAXCOLS];	/* stdscr, mw and cw */
int save_lines, save_cols;		/* and how big they were */

#ifdef __SANITIZE_ADDRESS__
#define NOCHECK	__attribute__((no_sanitize_address))
#define read	rg_read
#else
#define NOCHECK
#endif

STAT sbuf;

/*
 * save_ok:
 *	A saved game is the game's variables from version to rg_edata
 *	and from rg_bss to rg_ebss.  See that they are where that takes
 *	them to be, by one variable of each kind, and that save_win[]
 *	has room for the screen.
 */
save_ok()
{
    return ((char *) &foodlev >= version && (char *) &foodlev < rg_edata
	&& (char *) &save_lines >= rg_bss && (char *) &save_lines < rg_ebss
	&& LINES <= MAXLINES && COLS <= MAXCOLS);
}

/*
 * save_wins:
 *	Copy what is in the windows that hold the level to save_win[],
 *	or back from it into the windows
 */
save_wins(back)
bool back;
{
    register int i, y, x;
    register WINDOW *win;

    for (i = 0; i < 3; i++)
    {
	win = (i == 0 ? stdscr : i == 1 ? mw : cw);
	for (y = 0; y < LINES; y++)
	    for (x = 0; x < COLS; x++)
		if (back)
		    mvwaddch(win, y, x, (unsigned char) save_win[i][y][x]);
		else
		    save_win[i][y][x] = mvwinch(win, y, x) & A_CHARTEXT;
    }
}

save_game()
{
    register int savefd;
    register int c;
    char buf[2*LINELEN];

    if (!save_ok())
    {
	msg("This rogue cannot save a game");
	return FALSE;
    }
    /*
     * get file name
     */
    mpos = 0;
    if (file_name[0] != '\0') {
	msg("Save file (%s)? ", file_name);
	do {
	    c = readchar();
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
	if ((savefd = save_open()) == -1)
	    msg(strerror(errno));	/* fake perror() */
    } while (savefd == -1);

    /*
     * write out encrpyted file (after a stat)
     * The fwrite is to force allocation of the buffer before the write
     */
    save_file(savefd);
    return TRUE;
}

/*
 * save_open:
 *	Make the file to save the game in, new, and closed to everyone
 *	but the player.  A regular file in the way is removed first, and
 *	anything else there makes it fail.
 */
save_open()
{
    STAT sb;

    if (stat(file_name, &sb) >= 0 && (sb.st_mode & S_IFMT) == S_IFREG)
	unlink(file_name);
    return open(file_name, O_WRONLY|O_CREAT|O_EXCL, 0400);
}

save_file(savefd)
register int savefd;
{
    wmove(cw, LINES-1, 0);
    wrefresh(cw);
    fstat(savefd, &sbuf);
    write(savefd, "junk", 5);
    lseek(savefd, 0L, 0);
    save_lines = LINES;
    save_cols = COLS;
    save_wins(FALSE);
    save_stamp[0] = (long) playit - (long) version;
    save_stamp[1] = (long) rg_restored - (long) __executable_start;
    encwrite(version, rg_edata - version, savefd);
    encwrite(rg_bss, rg_ebss - rg_bss, savefd);
    close(savefd);
}

restore(file, envp)
register char *file;
char **envp;
{
    register int infd;
    extern char **environ;
    register char *sp;
    char buf[2*LINELEN], name[2*LINELEN];
    STAT sbuf2;
    int y, x, owiz;

    if (strcmp(file, "-r") == 0)
	file = strcpy(name, file_name);
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
    if (sbuf2.st_size != save_size())
    {
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }
    fflush(stdout);
    lseek(infd, 0L, 0);
    owiz = wizard;
    if (encread(version, rg_edata - version, infd) != rg_edata - version
	|| strcmp(buf, version) != 0
	|| encread(rg_bss, rg_ebss - rg_bss, infd) != rg_ebss - rg_bss)
    {
	printf("Sorry, saved game is damaged.\n");
	return FALSE;
    }
    if (save_stamp[0] != (long) playit - (long) version
	|| save_stamp[1] != (long) rg_restored - (long) __executable_start)
    {
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }
    rg_rebase(__executable_start, _end, version, rg_edata, rg_bss, rg_ebss);
    if (!rg_restored())
    {
	printf("Sorry, saved game is damaged.\n");
	return FALSE;
    }
    if ((wizard = owiz) != FALSE)
	waswizard = TRUE;

    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    initscr();
    getmaxyx(stdscr, y, x);
    if (y < save_lines || x < save_cols)
    {
	endwin();
	printf("Sorry, this saved game needs a terminal with %d lines and %d columns.\n", save_lines, save_cols);
	return FALSE;
    }
    LINES = save_lines;
    COLS = save_cols;
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    save_wins(TRUE);
    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    if (unlink(file) < 0) {
	endwin();
	printf("Cannot unlink file\n");
	return FALSE;
    }

    environ = envp;
    if ((sp = getenv("OPTIONS")) != NULL)
	parse_opts(sp);

    strcpy(file_name, file);

    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
    noecho();
    nonl();
    srand(1234);
    playit();
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
 *	unsigned char *start; long size; int outfd;
 *
 *	encread (start, size, infd);
 *	unsigned char *start; long size; int infd;
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
# define OFFSET		667818L
# define MODULUS	894871L
# define MULT		2399L
# define ENCHAR		((unsigned char) ((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10))

/* Constants for checksumming */
# define INITCK		1232531L
# define CKMOD		6506347L

struct wb { int w1, w2, w3, w4; };

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/

NOCHECK
encwrite (start, size, outfd)
register unsigned char *start;
long size;
int outfd;
{
  register unsigned char *buf_p, *end_p;
  register int ch;
  register long cksum = INITCK, cnt, seed;
  char buffer[512 * 20];

  srandom (time ((long *) 0));	/* Build a random seed */
  seed = (random () & 0x7fff);
  
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

NOCHECK
encread (start, size, infd)
register unsigned char *start;
long size;
int infd;
{ register int length, ch;
  int stored;
  long seed, cnt, getword(), cksum = INITCK;

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
long word;
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

long
getword (fd)
int fd;
{ struct wb w;
  
  if (read (fd, (char *) &w, sizeof w) == sizeof w)
    return (w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
  else
    return (0);
}

/*
 * save_size:
 *	How long a game saved by this build is: the two pieces of the
 *	game's data, each with the seed, length and checksum encwrite()
 *	puts around it
 */
long
save_size()
{
    return (rg_edata - version) + (rg_ebss - rg_bss) + 6 * sizeof (struct wb);
}
