/* PDCurses */

#include <curspriv.h>

/*man-start**************************************************************

touch
-----

### Synopsis

    int touchwin(WINDOW *win);
    int touchline(WINDOW *win, int start, int count);
    int untouchwin(WINDOW *win);
    int wtouchln(WINDOW *win, int y, int n, int changed);
    bool is_linetouched(WINDOW *win, int line);
    bool is_wintouched(WINDOW *win);

    int touchoverlap(const WINDOW *win1, WINDOW *win2);

### Description

   touchwin() and touchline() throw away all information about which
   parts of the window have been touched, pretending that the entire
   window has been drawn on. This is sometimes necessary when using
   overlapping windows, since a change to one window will affect the
   other window, but the records of which lines have been changed in the
   other window will not reflect the change.

   untouchwin() marks all lines in the window as unchanged since the
   last call to wrefresh().

   wtouchln() makes n lines in the window, starting at line y, look as
   if they have (changed == 1) or have not (changed == 0) been changed
   since the last call to wrefresh().

   is_linetouched() returns TRUE if the specified line in the specified
   window has been changed since the last call to wrefresh().

   is_wintouched() returns TRUE if the specified window has been changed
   since the last call to wrefresh().

   touchoverlap(win1, win2) marks the portion of win2 which overlaps
   with win1 as modified.

### Return Value

   All functions return OK on success and ERR on error except
   is_wintouched() and is_linetouched().

### Portability
                             X/Open  ncurses  NetBSD
    touchwin                    Y       Y       Y
    touchline                   Y       Y       Y
    untouchwin                  Y       Y       Y
    wtouchln                    Y       Y       Y
    is_linetouched              Y       Y       Y
    is_wintouched               Y       Y       Y
    touchoverlap                -       -       Y

**man-end****************************************************************/

int touchwin(WINDOW *win)
{
    int i;

    PDC_LOG(("touchwin() - called: Win=%x\n", win));

    if (!win)
        return ERR;

    for (i = 0; i < win->_maxy; i++)
    {
        win->_firstch[i] = 0;
        win->_lastch[i] = win->_maxx - 1;
    }

    return OK;
}

int touchline(WINDOW *win, int start, int count)
{
    int i;

    PDC_LOG(("touchline() - called: win=%p start %d count %d\n",
             win, start, count));

    if (!win || start > win->_maxy || start + count > win->_maxy)
        return ERR;

    for (i = start; i < start + count; i++)
    {
        win->_firstch[i] = 0;
        win->_lastch[i] = win->_maxx - 1;
    }

    return OK;
}

int untouchwin(WINDOW *win)
{
    int i;

    PDC_LOG(("untouchwin() - called: win=%p", win));

    if (!win)
        return ERR;

    for (i = 0; i < win->_maxy; i++)
    {
        win->_firstch[i] = _NO_CHANGE;
        win->_lastch[i] = _NO_CHANGE;
    }

    return OK;
}

int wtouchln(WINDOW *win, int y, int n, int changed)
{
    int i;

    PDC_LOG(("wtouchln() - called: win=%p y=%d n=%d changed=%d\n",
             win, y, n, changed));

    if (!win || y > win->_maxy || y + n > win->_maxy)
        return ERR;

    for (i = y; i < y + n; i++)
    {
        if (changed)
        {
            win->_firstch[i] = 0;
            win->_lastch[i] = win->_maxx - 1;
        }
        else
        {
            win->_firstch[i] = _NO_CHANGE;
            win->_lastch[i] = _NO_CHANGE;
        }
    }

    return OK;
}

bool is_linetouched(WINDOW *win, int line)
{
    PDC_LOG(("is_linetouched() - called: win=%p line=%d\n", win, line));

    if (!win || line > win->_maxy || line < 0)
        return FALSE;

    return (win->_firstch[line] != _NO_CHANGE) ? TRUE : FALSE;
}

bool is_wintouched(WINDOW *win)
{
    int i;

    PDC_LOG(("is_wintouched() - called: win=%p\n", win));

    if (win)
        for (i = 0; i < win->_maxy; i++)
            if (win->_firstch[i] != _NO_CHANGE)
                return TRUE;

    return FALSE;
}

int touchoverlap(const WINDOW *win1, WINDOW *win2)
{
    int y, endy, endx, starty, startx;

    PDC_LOG(("touchoverlap() - called: win1=%p win2=%p\n", win1, win2));

    if (!win1 || !win2)
        return ERR;

    starty = max(win1->_begy, win2->_begy);
    startx = max(win1->_begx, win2->_begx);
    endy = min(win1->_maxy + win1->_begy, win2->_maxy + win2->_begy);
    endx = min(win1->_maxx + win1->_begx, win2->_maxx + win2->_begx);

    if (starty >= endy || startx >= endx)
        return OK;

    starty -= win2->_begy;
    startx -= win2->_begx;
    endy -= win2->_begy;
    endx -= win2->_begx;
    endx -= 1;

    for (y = starty; y < endy; y++)
    {
        win2->_firstch[y] = startx;
        win2->_lastch[y] = endx;
    }

    return OK;
}
