/*
 * Last Modified: 07/30/86
 */
/*
 * Note:  In this file, we use the system curses.h file
 * rather that our own version.
 *
 * random and associated routines lifted from 4.2BSD
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

#if defined(NRANDOM)
#define		TYPE_0		0		/* linear congruential */
#define		BREAK_0		8
#define		DEG_0		0
#define		SEP_0		0

#define		TYPE_1		1		/* x**7 + x**3 + 1 */
#define		BREAK_1		32
#define		DEG_1		7
#define		SEP_1		3

#define		TYPE_2		2		/* x**15 + x + 1 */
#define		BREAK_2		64
#define		DEG_2		15
#define		SEP_2		1

#define		TYPE_3		3		/* x**31 + x**3 + 1 */
#define		BREAK_3		128
#define		DEG_3		31
#define		SEP_3		3

#define		TYPE_4		4		/* x**63 + x + 1 */
#define		BREAK_4		256
#define		DEG_4		63
#define		SEP_4		1

#define		MAX_TYPES	5		/* max number of types above */

static  int		degrees[ MAX_TYPES ]	= { DEG_0, DEG_1, DEG_2,
								DEG_3, DEG_4 };

static  int		seps[ MAX_TYPES ]	= { SEP_0, SEP_1, SEP_2,
								SEP_3, SEP_4 };

static  long		randtbl[ DEG_3 + 1 ]	= { TYPE_3,
			    0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342, 
			    0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb, 
			    0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd, 
			    0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86, 
			    0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7, 
			    0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc, 
			    0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b, 
					0xf5ad9d0e, 0x8999220b, 0x27fb47b9 };

static  long		*fptr			= &randtbl[ SEP_3 + 1 ];
static  long		*rptr			= &randtbl[ 1 ];

static  long		*state			= &randtbl[ -1 ];

static  int		rand_type		= TYPE_3;
static  int		rand_deg		= DEG_3;
static  int		rand_sep		= SEP_3;

static  long		*end_ptr		= &randtbl[ DEG_3 + 1 ];

srandom( x )

    unsigned		x;
{
	long random();
    	register  int		i, j;

	if(  rand_type  ==  TYPE_0  )  {
	    state[ 0 ] = x;
	}
	else  {
	    j = 1;
	    state[ 0 ] = x;
	    for( i = 1; i < rand_deg; i++ )  {
		state[i] = 1103515245*state[i - 1] + 12345;
	    }
	    fptr = &state[ rand_sep ];
	    rptr = &state[ 0 ];
	    for( i = 0; i < 10*rand_deg; i++ )  random();
	}
}

char  *
initstate( seed, arg_state, n )

    unsigned		seed;			/* seed for R. N. G. */
    char		*arg_state;		/* pointer to state array */
    int			n;			/* # bytes of state info */
{
	register  char		*ostate		= (char *)( &state[ -1 ] );

	if(  rand_type  ==  TYPE_0  )  state[ -1 ] = rand_type;
	else  state[ -1 ] = MAX_TYPES*(rptr - state) + rand_type;
	if(  n  <  BREAK_1  )  {
	    if(  n  <  BREAK_0  )  {
		fprintf( stderr, "initstate: not enough state (%d bytes) with which to do jack; ignored.\n" );
		return;
	    }
	    rand_type = TYPE_0;
	    rand_deg = DEG_0;
	    rand_sep = SEP_0;
	}
	else  {
	    if(  n  <  BREAK_2  )  {
		rand_type = TYPE_1;
		rand_deg = DEG_1;
		rand_sep = SEP_1;
	    }
	    else  {
		if(  n  <  BREAK_3  )  {
		    rand_type = TYPE_2;
		    rand_deg = DEG_2;
		    rand_sep = SEP_2;
		}
		else  {
		    if(  n  <  BREAK_4  )  {
			rand_type = TYPE_3;
			rand_deg = DEG_3;
			rand_sep = SEP_3;
		    }
		    else  {
			rand_type = TYPE_4;
			rand_deg = DEG_4;
			rand_sep = SEP_4;
		    }
		}
	    }
	}
	state = &(  ( (long *)arg_state )[1]  );	/* first location */
	end_ptr = &state[ rand_deg ];	/* must set end_ptr before srandom */
	srandom( seed );
	if(  rand_type  ==  TYPE_0  )  state[ -1 ] = rand_type;
	else  state[ -1 ] = MAX_TYPES*(rptr - state) + rand_type;
	return( ostate );
}

char  *
setstate( arg_state )

    char		*arg_state;
{
	register  long		*new_state	= (long *)arg_state;
	register  int		type		= new_state[0]%MAX_TYPES;
	register  int		rear		= new_state[0]/MAX_TYPES;
	char			*ostate		= (char *)( &state[ -1 ] );

	if(  rand_type  ==  TYPE_0  )  state[ -1 ] = rand_type;
	else  state[ -1 ] = MAX_TYPES*(rptr - state) + rand_type;
	switch(  type  )  {
	    case  TYPE_0:
	    case  TYPE_1:
	    case  TYPE_2:
	    case  TYPE_3:
	    case  TYPE_4:
		rand_type = type;
		rand_deg = degrees[ type ];
		rand_sep = seps[ type ];
		break;

	    default:
		fprintf( stderr, "setstate: state info has been munged; not changed.\n" );
	}
	state = &new_state[ 1 ];
	if(  rand_type  !=  TYPE_0  )  {
	    rptr = &state[ rear ];
	    fptr = &state[ (rear + rand_sep)%rand_deg ];
	}
	end_ptr = &state[ rand_deg ];		/* set end_ptr too */
	return( ostate );
}

long
random()
{
	long		i;
	
	if(  rand_type  ==  TYPE_0  )  {
	    i = state[0] = ( state[0]*1103515245 + 12345 )&0x7fffffff;
	}
	else  {
	    *fptr += *rptr;
	    i = (*fptr >> 1)&0x7fffffff;	/* chucking least random bit */
	    if(  ++fptr  >=  end_ptr  )  {
		fptr = state;
		++rptr;
	    }
	    else  {
		if(  ++rptr  >=  end_ptr  )  rptr = state;
	    }
	}
	return( i );
}


get_rand(x, y)
int x, y;
{
	long r, random();
	int s;

	two_sort(x, y);

	r = random();
	r = (r % ((y-x)+1)) + x;
	s = (int) r;
	return(s);
}

rand_percent(percentage)
{
	return(get_rand(1, 100) <= percentage);
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
