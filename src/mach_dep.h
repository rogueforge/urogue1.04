/*
 * Last Modified: 05/25/86
 */
/*
 * machine dependencies
 *
 */

#include "defs.h"

#if	defined(BSD4)
#define	strchr	index
#define	strrchr	rindex
#define srand48	srandom
#define lrand48	random
#endif

#if	defined(USGV3)
#define	srand48	srand
#define	lrand48	rand
#define	_doprnt	_dprnt
#define	setegid	setgid
#define	seteuid	setuid
#endif

#if	defined(RTU2)
#define	srand48	srand
#define	lrand48	rand
#define setegid	setgid
#define	seteuid	setuid
#endif

#if	defined(USGV5)
#define setegid	setgid
#define	seteuid	setuid
#if	defined(RTU3)
#define	srand48	srandom
#define	lrand48	random
#endif
#endif
