/*
 * Last Modified: 05/25/86
 */
/*
 * This is a hack,  pure and simple to allow for
 * tty magic in BSD unix (BSD4)
 *
 * I basically need to have all calls that change the tty state
 * to pass though my hands (in sys_dep.c).
 */

#include	"/usr/include/curses.h"

#if	defined(BSD4)
/*
 * Set these routines to point to my routines
 */
#undef raw
#undef noraw
#undef crmode
#undef nocrmode
#undef echo
#undef noecho
#undef nl
#undef nonl
#undef  savetty
#undef  resetty

#undef  initscr
#undef  endwin

#define raw     myraw
#define noraw   mynoraw
#define crmode  mycrmode
#define nocrmode	mynocrmode
#define echo    myecho
#define noecho  mynoecho
#define nl      mynl
#define nonl    mynonl
#define savetty mysavetty
#define resetty myresetty

#define initscr myinitscr
#define endwin  myendwin
#else
#if defined(attron)
#if !defined(reg)
#define		reg		register
#endif reg
#define		_tty		cur_term->Ottyb
#endif attron
#endif BSD4
