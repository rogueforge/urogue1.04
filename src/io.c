/*
 * Last Modified: 08/10/86
 */
/*
 * Various input/output functions
 */

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include "rogue.h"
#include "mach_dep.h"
#include "stepok.h"
#include "varargs.h"

static char mbuf[2*BUFSIZ];
static int newpos = 0;

/*
 * msg:
 *	Display a message at the top of the screen.
 */


/*VARARGS1*/
msg(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list ap;


    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0') {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	return;
    }
    /*
     * otherwise add to the message and flush it out
     */
    va_start(ap);
    doadd(fmt, ap);
    va_end(ap);
    endmsg();
}

/*
 * add things to the current message
 */
/*VARARGS1*/
addmsg(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list ap;

    va_start(ap);
    doadd(fmt, ap);
    va_end(ap);
}

/*
 * Display a new msg (giving him a chance to see the previous one if it
 * is up there with the --More--)
 */
endmsg()
{
    strcpy(msgbuf[msg_index], mbuf);
    msg_index = ++msg_index % 10;
    if (mpos) {
	wmove(cw, 0, mpos);
	waddstr(cw, morestr);
	wrefresh(cw);
	wait_for(' ');
    }
    mvwaddstr(cw, 0, 0, mbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
    wrefresh(cw);
}

/*VARARGS1*/
doadd(fmt, args)
char *fmt;
int **args;
{
    static FILE junk;

    /*
     * Do the printf into buf
     */
    junk._flag = _IOWRT;  /* Under 3.0 IOUNK was IOSTRG */
    junk._ptr = &mbuf[newpos];
#if defined(STDIO_BASE_P)
    junk._base = junk._ptr;
#endif STDIO_BASE_P
#if defined(_NFILE)
    junk._file = _NFILE;
#endif _NFILE
    junk._cnt = 32767;
    _doprnt(fmt, args, &junk);
    putc('\0', &junk);
    newpos = strlen(mbuf);
}

/*
 * step_ok:
 *	returns true if it is ok for type to step on ch
 *	flgptr will be NULL if we don't know what the monster is yet!
 */

step_ok(y, x, can_on_monst, flgptr)
register int y, x, can_on_monst;
register struct thing *flgptr;
{
    register struct linked_list *item;
    char ch;

    /* What is here?  Don't check monster window if MONSTOK is set */
    if (can_on_monst == MONSTOK) 
	ch = mvinch(y, x);
    else
	ch = winat(y, x);

    switch (ch) {
	when ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    if (flgptr && on(*flgptr, CANINWALL))
		return(TRUE);
	    return FALSE;
	when SCROLL:
	    /*
	     * If it is a scroll, it might be a scare monster scroll
	     * so we need to look it up to see what type it is.
	     */
	    if (flgptr && flgptr->t_ctype == C_MONSTER) {
		item = find_obj(y, x, TRUE);
		if (item != NULL && (OBJPTR(item))->o_type == SCROLL &&
		    (OBJPTR(item))->o_which == S_SCARE &&
		    rnd(flgptr->t_stats.s_intel) < 12)
			return(FALSE); /* All but smart ones are scared */
	    }
	    return(TRUE);
	otherwise:
	    return (!isalpha(ch));
    }
}
/*
 * shoot_ok:
 *	returns true if it is ok for type to shoot over ch
 */

shoot_ok(ch)
{
    switch (ch) {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar:
 *	flushes stdout so that screen is up to date and then returns
 *	getchar.
 */

readchar()
{
    char c;

    fflush(stdout);
    while (read(0, &c, 1) < 0)
	;
    return (c & 0177);
}

#if !defined(unctrl)
/*
 * unctrl:
 *	Print a readable version of a certain character
 */

char *
unctrl(ch)
char ch;
{
    extern char *_unctrl[];		/* Defined in curses library */

    return _unctrl[ch&0177];
}
#endif

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

status(display)
bool display;	/* is TRUE, display unconditionally */
{
    register struct stats *stat_ptr, *max_ptr;
    register int oy, ox;
    static char buf[2*LINELEN];

    stat_ptr = &pstats;
    max_ptr  = &max_stats;

    getyx(cw, oy, ox);
    sprintf(buf, "Int:%d(%d) Str:%d(%d) Wis:%d(%d) Dex:%d(%d) Con:%d(%d) Carry:%d(%d)",
	stat_ptr->s_intel, max_ptr->s_intel,
	stat_ptr->s_str, max_ptr->s_str,
	stat_ptr->s_wisdom,max_ptr->s_wisdom,
	stat_ptr->s_dext,max_ptr->s_dext,
	stat_ptr->s_const,max_ptr->s_const,
	stat_ptr->s_pack/10, stat_ptr->s_carry/10
	);
    mvwaddstr(cw, LINES - 2, 0, buf);
    wclrtoeol(cw);

    sprintf(buf, "Lvl:%d Au:%d Hpt:%d(%d) Pow:%d(%d) Ac:%d Exp:%d+%ld %s",
	level,
	purse,
	stat_ptr->s_hpt, max_ptr->s_hpt,
	stat_ptr->s_power,max_ptr->s_power,
	(cur_armor != NULL ? (cur_armor->o_ac - 10 + stat_ptr->s_arm)
		: stat_ptr->s_arm) - ring_value(R_PROTECT),
	stat_ptr->s_lvl,
	stat_ptr->s_exp,
	cnames[player.t_ctype][min(stat_ptr->s_lvl-1, 14)]);
    mvwaddstr(cw, LINES - 1, 0, buf);
    switch (hungry_state) {
	when F_OK: ;
	when F_HUNGRY:
	    waddstr(cw, " Hungry");
	when F_WEAK:
	    waddstr(cw, " Weak");
	when F_FAINT:
	    waddstr(cw, " Fainting");
    }
    wclrtoeol(cw);
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */

wait_for(ch)
register char ch;
{
    register char c;

    if (ch == '\n')
        while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    else
        while (readchar() != ch)
	    continue;
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */

show_win(scr, message)
register WINDOW *scr;
char *message;
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    wrefresh(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
 * restscr:
 *	Restores the screen to the terminal
 */
restscr(scr)
WINDOW *scr;
{
	clearok(scr,TRUE);
	touchwin(scr);
}
/*
 * add_line:
 *    Add a line to the list of discoveries
 */

/* VARARGS1 */
add_line(fmt, arg)
char *fmt, *arg;
{
    register WINDOW *tw;

    if (line_cnt == 0) {
        wclear(hw);
        if (inv_type == INV_SLOW)
            mpos = 0;
    }
    if (inv_type == INV_SLOW) {
        if (*fmt != '\0')
            msg(fmt, arg);
        line_cnt++;
    } else {
        /* chai: was if (line_cnt >= LINES - 1 || fmt == NULL) { */
        if (line_cnt >= LINES - 2 || fmt == NULL) {
            if (fmt == NULL && !newpage && inv_type == INV_OVER) {
                tw = subwin(hw, line_cnt + 1, COLS, 0, 0);
/*                overwrite(hw, tw);*/
                wstandout(tw);
                mvwaddstr(tw, line_cnt, 0, spacemsg);
                wstandend(tw);
                touchwin(tw);
                wrefresh(tw);
                wait_for(' ');
#if defined(BSD4)
                if (CE) {
                    werase(tw);
                    wrefresh(tw);
                }
#endif
                delwin(tw);
                touchwin(cw);
            } else {
                wstandout(hw);
                mvwaddstr(hw, LINES - 1, 0, spacemsg);
                wstandend(hw);
                wrefresh(hw);
                wait_for(' ');
                clearok(curscr, TRUE);
                wclear(hw);
            }
            newpage = TRUE;
            line_cnt = 0;
        }
        if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0')) {
            mvwprintw(hw, line_cnt++, 0, fmt, arg);
            lastfmt = fmt;
            lastarg = arg;
        }
    }
}

/*
 * end_line:
 *    End the list of lines
 */
end_line()
{
    if (inv_type != INV_SLOW)
        if (line_cnt == 1 && !newpage) {
            mpos = 0;
            msg(lastfmt, lastarg);
        } else
            add_line(NULL);
    line_cnt = 0;
    newpage = FALSE;
}
