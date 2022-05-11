/* PDCurses */

#include <curspriv.h>

/*man-start**************************************************************

move
----

### Synopsis

    int move(int y, int x);
    int mvcur(int oldrow, int oldcol, int newrow, int newcol);
    int wmove(WINDOW *win, int y, int x);

### Description

   move() and wmove() move the cursor associated with the window to the
   given location. This does not move the physical cursor of the
   terminal until refresh() is called. The position specified is
   relative to the upper left corner of the window, which is (0,0).

   mvcur() moves the physical cursor without updating any window cursor
   positions.

### Return Value

   All functions return OK on success and ERR on error.

### Portability
                             X/Open  ncurses  NetBSD
    move                        Y       Y       Y
    mvcur                       Y       Y       Y
    wmove                       Y       Y       Y

**man-end****************************************************************/

int move(int y, int x)
{
    PDC_LOG(("move() - called: y=%d x=%d\n", y, x));

    if (!stdscr || x < 0 || y < 0 || x >= stdscr->_maxx || y >= stdscr->_maxy)
        return ERR;

    stdscr->_curx = x;
    stdscr->_cury = y;

    return OK;
}

int mvcur(int oldrow, int oldcol, int newrow, int newcol)
{
    PDC_LOG(("mvcur() - called: oldrow %d oldcol %d newrow %d newcol %d\n",
             oldrow, oldcol, newrow, newcol));

    if (!SP || newrow < 0 || newrow >= LINES || newcol < 0 || newcol >= COLS)
        return ERR;

    PDC_gotoyx(newrow, newcol);
    SP->cursrow = newrow;
    SP->curscol = newcol;

    return OK;
}

int wmove(WINDOW *win, int y, int x)
{
    PDC_LOG(("wmove() - called: y=%d x=%d\n", y, x));

    if (!win || x < 0 || y < 0 || x >= win->_maxx || y >= win->_maxy)
        return ERR;

    win->_curx = x;
    win->_cury = y;

    return OK;
}
