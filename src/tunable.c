/*
 * Last Modified: 05/25/86
 */
/*
 * Machine dependent (tunable) parameters.
 */
#define	ROGUEPATH(FILE)	"/usr/games/lib/urogue/FILE"

/*
 * Wizard's identity.
 */
int	UID		=	696;			/* his user id */
char	LOGIN[]		=	"shadow";		/* his login name */

/*
 * Special files.
 */

/*
 * File containing uids of people considered authors
 */
char	PERMOK[]	=	ROGUEPATH(permok);

/*
 * File containing wizard's password and password key.
 */ 
char	PASSCTL[]	=	ROGUEPATH(passctl);

/*
 * File where news is kept
 */
char	NEWS[]		=	ROGUEPATH(news);

/*
 * File where top ten scores are kept
 */
char	SCOREDIR[]	=	ROGUEPATH(scorefile);

/*
 * Where MOTD file is
 */
char	MOTD[]	=		ROGUEPATH(motd);

/*
 * Where file is that disables playing of urogue and whose contents
 * are printed instead
 */
char	NOPLAY[]=		ROGUEPATH(noplay);

/*
 * Location of lav program to print load averages
 */
char	LOADAV_PROG[]	=	"/usr/ucb/uptime";

/*
 * String to pass to scanf to get the instant, 5 min, and 15 min load ave
 * Following string is generic uptime parser
 */
char	LOADAV_SCAN[]	=	"%*[^:]:%lf%*[, ]%lf%*[, ]%lf";

/*
 * Name of environment variable to check for options in
 */
char	OPTIONS[]	=	"UROGUE";

/*
 * Location of utmp
 */
char	UTMP[]		=	"/etc/utmp";

/*
 * Name of player definitions file
 */
char	ROGDEFS[]	=	".rogdefs";

/*
 * default name of saved game file
 */
char	ROGUE_SAVE[]	=	"urogue.save";

/*
 * Name of pager routine for news and motd
 */
char	PAGER[]		=	"/bin/cat";

/*
 * Load control constants
 */
int	MAXLOAD		=	300;	/* 10 * max 15 minute load average */
int	MAXUSERS	=	50;	/* max users at start */
int	HOLIDAY		=	0;	/* enable prime shift restrictions */
int	CHECKTIME	=	150;	/* minutes between load checks */
int	SAVETIME	=	180;	/* seconds allowed to save game */
int	WARNTIME	=	120;	/* seconds between load warnings */
