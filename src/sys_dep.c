/*
 * Last Modified: 07/30/86
 */
/*
 * Note:  In this file, we use the system curses.h file
 * rather that our own version.
 */


#include <curses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "defs.h"
#include "mach_dep.h"
#if defined(BSD4)
#include <signal.h>
#endif

#if defined(USGV5)
extern int _tty_ch;
#if !defined(attron) && !defined(PC7300)
extern SGTTY _tty;
#endif NOT attron
#endif

#if !defined(flushout) && !defined(USGV5)
flushout()
{
	int out = 1;

	ioctl(_tty_ch, TIOCFLUSH, (char *)&out);
}
#endif

#if defined(BSD4)

static  struct  ltchars OriginalLtchars;
static  struct  ltchars GameLtchars;

static  struct  tchars  OriginalTchars;
static  struct  tchars  GameTchars;

static  struct  sgttyb  OriginalSgttyb;
static  struct  sgttyb  GameSgttyb;

myinitscr()
{

  /*
   * I am assuming that the game runs off of stdin (ie fd 0)
   */

  if( initscr() == ERR ) {
    return 0;
  }
  if( TtyGetGameChars() && TtyMapGameChars() ) {
    return 1;
  } else {
    (void) TtySetOriginalChars();
    return 0;
  }
}

myendwin()
{

  (void) endwin();
  return TtySetOriginalChars();
}

myraw()
{

  extern struct sgttyb GameSgttyb;

  GameSgttyb.sg_flags |= RAW;
  return raw();
}

mynoraw()
{

  extern struct sgttyb GameSgttyb;

  GameSgttyb.sg_flags &= ~RAW;
  return noraw();
}

mycrmode()
{

  extern struct sgttyb GameSgttyb;

  GameSgttyb.sg_flags |= CBREAK;
  return crmode();
}

mynocrmode()
{

  extern struct sgttyb GameSgttyb;

  GameSgttyb.sg_flags &= ~CBREAK;
  return nocrmode();
}

myecho()
{

  extern	struct  sgttyb  GameSgttyb;

  GameSgttyb.sg_flags |= ECHO;
  return echo();
}

mynoecho()
{

  extern	struct  sgttyb  GameSgttyb;

  GameSgttyb.sg_flags &= ~ECHO;
  return noecho();
}

mynl()
{

  extern	struct  sgttyb  GameSgttyb;

  GameSgttyb.sg_flags |= CRMOD;
  return nl();
}

mynonl()
{

  extern	struct  sgttyb  GameSgttyb;

  GameSgttyb.sg_flags &= ~CRMOD;
  return nonl();
}

mysavetty()
{

  /*
   * If all calls have gone though us,
   * we should not ever need to do anything at this point.
   */

  return savetty();
}

myresetty()
{

  /*
   * If everything has gone well,  we can ignore this,
   * and just pass it on.
   */

  resetty();
}

TtyInit()
{

  if(
	!TtyGetOriginalChars() ||
	!TtyGetGameChars() ) {
	  return 0;
	}
  return 1;
}

TtyGetOriginalChars()
{
  extern	struct  ltchars OriginalLtchars;
  extern	struct  tchars  OriginalTchars;
  extern	struct  sgttyb  OriginalSgttyb;

  if(
	ioctl( 0, TIOCGLTC, (char *) &OriginalLtchars) == -1 ||
	ioctl( 0, TIOCGETC, (char *) &OriginalTchars) == -1 ||
	ioctl( 0, TIOCGETP, (char *) &OriginalSgttyb) == -1 ) {
	  return 0;
	}
  return 1;
}

TtySetOriginalChars()
{

  extern	struct  ltchars OriginalLtchars;
  extern	struct  tchars  OriginalTchars;
  extern	struct  sgttyb  OriginalSgttyb;

  if(
	ioctl( 0, TIOCSLTC, (char *) &OriginalLtchars) == -1 ||
	ioctl( 0, TIOCSETC, (char *) &OriginalTchars) == -1 ||
	ioctl( 0, TIOCSETP, (char *) &OriginalSgttyb) == -1 ) {
	  return 0;
	}
  return 1;
}

TtyGetGameChars()
{
  extern	struct  ltchars GameLtchars;
  extern	struct  tchars  GameTchars;
  extern	struct  sgttyb  GameSgttyb;

  if(
	ioctl( 0, TIOCGLTC, (char *) &GameLtchars) == -1 ||
	ioctl( 0, TIOCGETC, (char *) &GameTchars) == -1 ||
	ioctl( 0, TIOCGETP, (char *) &GameSgttyb) == -1 ) {
	  return 0;
	}
  return 1;
}


TtySetGameChars()
{

  extern	struct  ltchars GameLtchars;
  extern	struct  tchars  GameTchars;
  extern	struct  sgttyb  GameSgttyb;

  if(
	ioctl( 0, TIOCSLTC, (char *) &GameLtchars) == -1 ||
	ioctl( 0, TIOCSETC, (char *) &GameTchars) == -1 ||
	ioctl( 0, TIOCSETP, (char *) &GameSgttyb) == -1 ) {
	  return 0;
	}
  return 1;
}


TtyMapGameChars()
{

/*
 * Map all the special chars to non existant ones.
 * Since curses insists on at least CBREAK mode,
 * we can ignore sgttyb.sg_erase and sgttyb.sg_kill
 * (all of the sgttyb struct)
 *
 */

  extern	struct  ltchars GameLtchars;
  extern	struct  tchars  GameTchars;

#if !defined(CTRL)
#define CTRL(x) (('x') & (~'@'))
#endif

  GameLtchars.t_dsuspc = -1;
  GameLtchars.t_rprntc = -1;
  GameLtchars.t_flushc = -1;
  GameLtchars.t_werasc = -1;
  GameLtchars.t_lnextc = -1;

  GameTchars.t_quitc = CTRL(\\);
  GameTchars.t_startc = CTRL(Q);
  GameTchars.t_stopc = CTRL(S);
  GameTchars.t_eofc = -1;
  GameTchars.t_brkc = -1;

  return TtySetGameChars();
}

void
tstop()
{

  /*
   * Since we are running with job control,
   * we can assume that the signals remain set after an interupt.
   */


  int   (*SigCont)();

  /*
   * We are basically commited at this point,
   * so ignore return codes,
   */

  (void) mvcur( 0, COLS-1, LINES-1, 0);
  (void) TtySetOriginalChars();
  /*
   * Give us a fresh line to work with
   */
  putc( '\n', stdout );
  SigCont = signal( SIGCONT, SIG_DFL );
  (void) kill( 0, SIGSTOP );
  /*
   * This is the point that the process will stop at
   */

  /*
   * Lets restart the world.
   */
  (void) signal( SIGCONT, SigCont );
  (void) TtyGetOriginalChars();
  (void) TtySetGameChars();
  (void) wrefresh( curscr );
  return;
}
#endif /* BSD4 */

#if !defined(BSD4)
_dprnt(fmt, argp, file)
register char *fmt;
register int *argp;
register FILE *file;
{       register char c, *p, *q;
        register int d, width, ndigit, radix;
        register unsigned n;
        int sign, decpt;
        char t[128], digits[10], zfill, rjust, ndfnd;
        char *ecvt(), *fcvt();
        struct {
                short unsigned length;
                char dtype;
                char class;
                char *ptr;
        } desc;
        p = &t[0];
        while (c = *fmt++)
                if (c != '%')
                        *p++ = c;
                else {
                        if (p != &t[0]) {
                                sout(t, p-t, 0, file);
                                p = &t[0];
                        }
                        rjust = 0;
                        ndigit = 0;
                        zfill = ' ';
                        if (*fmt == '-') {
                                rjust++;
                                fmt++;
                        }
                        if (*fmt == '0') {
                                zfill = '0';
                                fmt++;
                        }
                        if(*fmt != '*') { 
                                width = 0;
                                while ((d = *fmt++ - '0') >= 0 && d <= 9)
                                        width = width*10+d;
                        }
                        else {
                                fmt++;
                                d = *fmt++ - '0';
                                width = *argp++;
                        }
                        ndfnd = 0;
                        if ((d += '0') == '.') {
                            if(*fmt != '*') {
                                ndigit = 0;
                                while ((d = *fmt++ - '0') >= 0 && d <= 9) {
                                        ndfnd++;
                                        ndigit = ndigit*10+d;
                                }
                                d += '0';
                            }
                            else {
                                fmt++;
                                d = *fmt++;
                                ndfnd++;
                                ndigit = *argp++;
                            }
                        }
                        switch (d) {
                        case 'l':
                        case 'L':
                                switch(*fmt++) {
                                case 'o': goto loct;
                                case 'x': goto lhex;
                                case 'd': goto ldec;
                                case 'u': goto luns;
                                default:
                                        fmt--;
                                        goto uns;
                                }
                        case 'o':
                        case 'O':
                        loct:   radix = 8;
                                n = *argp++;
                                goto compute;
                        case 'x':
                        case 'X':
                        lhex:   radix = 16;
                                n = *argp++;
                                goto compute;
                        case 'd':
                        case 'D':
                        ldec:   radix = 10;
                        signed: if (*argp < 0) {
                                        *p++ = '-';
                                        n = -*argp++;
                                } else
                                        n = *argp++;
                                goto compute;

                        case 'u':
                        case 'U':
                        luns:
                        uns:    n = *argp++;
                                radix = 10;
                        compute:
                                if (n == 0 && ndigit == 0)
                                        *p++ = '0';
                                for (q = &digits[0]; n != 0; n = n/radix)
                                {
                                        d = n%radix;
                                        *q++ = d + (d<10?'0':'a'-10);
                                }
                                while (q > &digits[0])
                                        *p++ = *--q;
                                goto prbuf;

                        case 'c':
                                for (q = (char *)argp++, d = 0; d < 4; d++)
                                        if ((*p++ = *q++) == 0)
                                                p--;
                        prbuf:  q = &t[0];
                        prstr:  if ((d = width - (p - q)) < 0)
                                        d = 0;
                                if (rjust == 0)
                                        d = -d;
                                sout(q, p-q, d, file, zfill);
                                p = &t[0];
                                break;

                        case 's':
                                if ((q = (char *)*argp++) == 0)
                                        q = "(null)";
                                if ((d = ndigit) == 0)
                                        d = 32767;
                                for (p=q; *p!=0 && --d>=0; p++);
                                goto prstr;

                        case 'r':
                                argp = (int *)*argp;
                                fmt = (char *)*argp++;
                                break;
                        case 'f':
                                if (ndfnd == 0)
                                        ndigit = 6;
                                q = fcvt(*((double *)argp), ndigit,
                                        &decpt, &sign); argp += 2;
                                if (sign)
                                        *p++ = '-';
                                if ((d = decpt) <= 0)
                                        *p++ = '0';
                                else do {
                                                *p++ = *q++;
                                        } while (--d > 0);
                                if (d = ndigit)
                                        *p++ = '.';
                                if ((decpt = - decpt) > 0)
                                        while (--d >= 0) {
                                                *p++ = '0';
                                                if (--decpt <= 0)
                                                        break;
                                        }
                                if (d > 0)
                                        while (--d >= 0)
                                                *p++ = *q++;
                                goto prbuf;

                        case 'e':
                                if (ndfnd == 0)
                                        ndigit = 6;
                                else
                                        ndigit += 1;
                                q = ecvt(*((double *)argp), ndigit,
                                        &decpt, &sign); argp += 2;
                                if (sign)
                                        *p++ = '-';
                                if (*q == '0')
                                        decpt += 1;
                                *p++ = *q++;
                                *p++ = '.';
                                for (d = ndigit; --d > 0; *p++ = *q++);
                                *p++ = 'e';
                                decpt -= 1;
                                if (decpt >= 0)
                                        *p++ = '+';
                                else {
                                        *p++ = '-';
                                        decpt = -decpt;
                                }
                                *p++ = (unsigned)decpt/10+'0';
                                *p++ = (unsigned)decpt%10+'0';
                                goto prbuf;

                        case 'g':
                                if (ndfnd == 0)
                                        ndigit = 6;
                                gcvt(*((double *)argp), ndigit, p); argp +=2;
                                while (*p++ != 0);
                                p -= 1;
                                goto prbuf;
                        case '%':
                                *p++= '%';
                                break;
                        }
        }
        if (p != &t[0])
                sout(t, p-t, 0, file);
}

sout(string, count, adjust, file, fillch)
register char *string;
register count;
int adjust;
register FILE *file;
{
        while (adjust < 0) {
                if (*string=='-' && fillch=='0') {
                        putc(*string++, file);
                        count--;
                }
                putc(fillch, file);
                adjust++;
        }
        while (--count>=0)
                putc(*string++, file);
        while (adjust) {
                putc(fillch, file);
                adjust--;
        }
}
#endif /* !defined BSD4 */
