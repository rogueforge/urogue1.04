/*
 * Last Modified: 08/13/86
 */

/*
 * Things dealing with a terminal's keypad.
 */

#include "rogue.h"
#include <signal.h>

/*
 * Interpret ESCAPE sequences for keypad movement,
 * or cancel command.
 */
doescape()
{
    int escfinish();
    register char ch;

    /*
     * set an alarm in case we don't really
     * want a keypad command, but a cancel.
     */
    signal(SIGALRM, escfinish);
    alarm(2);

    ch = readchar();
    /*
     * 'O' - Probably a vt100/ANSI clone.
     * '?' - Probably a brain damaged vt52.
     */
    if(ch == 'O' || ch == '?') {
	switch (ch = readchar()) {
	    when 'u' : /* do a Fast movement */
	    {
		if((ch = readchar()) == ESCAPE) {
		    ch = readchar();
    		    if(ch == 'O' || ch == '?') {
		    	switch(ch = readchar()) {
		            when 'w' : do_run('y');	/* up & left 	*/
		            when 'x' : do_run('k');	/* up 		*/
		            when 'y' : do_run('u');	/* up & right 	*/
		            when 'v' : do_run('l');	/* right 	*/
		            when 's' : do_run('n');	/* down & right */
		            when 'r' : do_run('j');	/* down 	*/
		            when 'q' : do_run('b');	/* down & left 	*/
		            when 't' : do_run('h');	/* left 	*/
			    otherwise:
			        alarm(0);
			        msg("Unknown direction \"%c\".", ch);
			        return(FALSE);
		        }
		    }
		} else {
		    alarm(0);
		    msg("Unknown direction `%c'.", ch);
		    return(FALSE);
		}
	    }
	    when 'w' : do_move(-1, -1);	/* up & left 	*/
	    when 'x' : do_move(-1, 0);	/* up 		*/
	    when 'y' : do_move(-1, 1);	/* up & right 	*/
	    when 'v' : do_move(0, 1);	/* right 	*/
	    when 's' : do_move(1, 1);	/* down & right */
	    when 'r' : do_move(1, 0);	/* down 	*/
	    when 'q' : do_move(1, -1);	/* down & left 	*/
	    when 't' : do_move(0, -1);	/* left 	*/
	    otherwise:
		alarm(0);
		msg("Unknown direction '%c'.", ch);
		return(FALSE);
	}
    }
    /*
     * turn off alarm and clean up.
     */
    alarm(0);
    return(TRUE);
}

/*
 * Come here to finish things off for ESCAPE
 * things.
 */
escfinish()
{
    door_stop = FALSE;
    count = 0;
    return(FALSE);
}

/*
 * Turn the terminal's keypad applications mode on.
 */
keypadon()
{
   printf("\033=");
}

/*
 * Turn off keypad mode - put terminal back into
 * numeric mode.
 */
keypadoff()
{
    printf("\033>");
}

static char *keypad[] = {
"+-------------+---------+--------------+",
"|  left & up  |   up    |  right & up  |",
"|       7     |    8    |      9       |",
"+-------------+---------+--------------+",
"|     left    |  <RUN>  |    right     |",
"|       4     |    5    |      6       |",
"+-------------+---------+--------------+",
"| left & down |  down   | right & down |",
"|       1     |    2    |      3       |",
"+-------------+---------+--------------+",
0 };

static char *khelpmsg[] = {
"    To use Key Pad Movement, you need a terminal that can generate",
"ANSI codes for ``auxilary'' or ``applications'' mode. These common",
"terminals include VT100's and most of the VT100 clones.",
" ",
"    To move in a direction, just type the appropriate key.  To run",
"in a particuliar direction, hit the <RUN> key and then the direction.",
0 };

/*
 * Print help info for key pad mode
 * Usually called from help().
 */
keypadhelp()
{
    register int i, x;
    static char *title = " K E Y   P A D   M O V E M E N T ";

    wclear(hw);
    wmove(hw, 0, (COLS - strlen(title)) / 2);
    wstandout(hw);
    wprintw(hw, title);
    wstandend(hw);

    wrefresh(hw);
    i = 0;
    while(keypad[i]) {
        wmove(hw,  2 + i, (COLS - strlen(keypad[i])) / 2);
	wprintw(hw, keypad[i]);
	++i;
    }
    wrefresh(hw);

    x = 0;
    while(khelpmsg[x]) {
	wmove(hw, x + 3 + i, 7);
	wprintw(hw, khelpmsg[x]);
	++x;
    }
    wrefresh(hw);
    wmove(hw, LINES - 1, 0);
    wprintw(hw, spacemsg);
    wrefresh(hw);
    wait_for(' ');
    wclear(hw);
}
