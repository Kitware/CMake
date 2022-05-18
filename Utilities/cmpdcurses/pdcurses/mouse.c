/* PDCurses */

#include <curspriv.h>

/*man-start**************************************************************

mouse
-----

### Synopsis

    int mouse_set(mmask_t mbe);
    int mouse_on(mmask_t mbe);
    int mouse_off(mmask_t mbe);
    int request_mouse_pos(void);
    void wmouse_position(WINDOW *win, int *y, int *x);
    mmask_t getmouse(void);

    int mouseinterval(int wait);
    bool wenclose(const WINDOW *win, int y, int x);
    bool wmouse_trafo(const WINDOW *win, int *y, int *x, bool to_screen);
    bool mouse_trafo(int *y, int *x, bool to_screen);
    mmask_t mousemask(mmask_t mask, mmask_t *oldmask);
    int nc_getmouse(MEVENT *event);
    int ungetmouse(MEVENT *event);
    bool has_mouse(void);

### Description

   As of PDCurses 3.0, there are two separate mouse interfaces: the
   classic interface, which is based on the undocumented Sys V mouse
   functions; and an ncurses-compatible interface. Both are active at
   all times, and you can mix and match functions from each, though it's
   not recommended. The ncurses interface is essentially an emulation
   layer built on top of the classic interface; it's here to allow
   easier porting of ncurses apps.

   The classic interface: mouse_set(), mouse_on(), mouse_off(),
   request_mouse_pos(), wmouse_position(), and getmouse(). An
   application using this interface would start by calling mouse_set()
   or mouse_on() with a non-zero value, often ALL_MOUSE_EVENTS. Then it
   would check for a KEY_MOUSE return from getch(). If found, it would
   call request_mouse_pos() to get the current mouse status.

   mouse_set(), mouse_on() and mouse_off() are analagous to attrset(),
   attron() and attroff(). These functions set the mouse button events
   to trap. The button masks used in these functions are defined in
   curses.h and can be or'ed together. They are the group of masks
   starting with BUTTON1_RELEASED.

   request_mouse_pos() requests curses to fill in the Mouse_status
   structure with the current state of the mouse.

   wmouse_position() determines if the current mouse position is within
   the window passed as an argument. If the mouse is outside the current
   window, -1 is returned in the y and x arguments; otherwise the y and
   x coordinates of the mouse (relative to the top left corner of the
   window) are returned in y and x.

   getmouse() returns the current status of the trapped mouse buttons as
   set by mouse_set() or mouse_on().

   The ncurses interface: mouseinterval(), wenclose(), wmouse_trafo(),
   mouse_trafo(), mousemask(), nc_getmouse(), ungetmouse() and
   has_mouse(). A typical application using this interface would start
   by calling mousemask() with a non-zero value, often ALL_MOUSE_EVENTS.
   Then it would check for a KEY_MOUSE return from getch(). If found, it
   would call nc_getmouse() to get the current mouse status.

   mouseinterval() sets the timeout for a mouse click. On all current
   platforms, PDCurses receives mouse button press and release events,
   but must synthesize click events. It does this by checking whether a
   release event is queued up after a press event. If it gets a press
   event, and there are no more events waiting, it will wait for the
   timeout interval, then check again for a release. A press followed by
   a release is reported as BUTTON_CLICKED; otherwise it's passed
   through as BUTTON_PRESSED. The default timeout is 150ms; valid values
   are 0 (no clicks reported) through 1000ms. In x11, the timeout can
   also be set via the clickPeriod resource. The return value from
   mouseinterval() is the old timeout. To check the old value without
   setting a new one, call it with a parameter of -1. Note that although
   there's no classic equivalent for this function (apart from the
   clickPeriod resource), the value set applies in both interfaces.

   wenclose() reports whether the given screen-relative y, x coordinates
   fall within the given window.

   wmouse_trafo() converts between screen-relative and window-relative
   coordinates. A to_screen parameter of TRUE means to convert from
   window to screen; otherwise the reverse. The function returns FALSE
   if the coordinates aren't within the window, or if any of the
   parameters are NULL. The coordinates have been converted when the
   function returns TRUE.

   mouse_trafo() is the stdscr version of wmouse_trafo().

   mousemask() is nearly equivalent to mouse_set(), but instead of
   OK/ERR, it returns the value of the mask after setting it. (This
   isn't necessarily the same value passed in, since the mask could be
   altered on some platforms.) And if the second parameter is a non-null
   pointer, mousemask() stores the previous mask value there. Also,
   since the ncurses interface doesn't work with PDCurses' BUTTON_MOVED
   events, mousemask() filters them out.

   nc_getmouse() returns the current mouse status in an MEVENT struct.
   This is equivalent to ncurses' getmouse(), renamed to avoid conflict
   with PDCurses' getmouse(). But if you define PDC_NCMOUSE before
   including curses.h, it defines getmouse() to nc_getmouse(), along
   with a few other redefintions needed for compatibility with ncurses
   code. nc_getmouse() calls request_mouse_pos(), which (not getmouse())
   is the classic equivalent.

   ungetmouse() is the mouse equivalent of ungetch(). However, PDCurses
   doesn't maintain a queue of mouse events; only one can be pushed
   back, and it can overwrite or be overwritten by real mouse events.

   has_mouse() reports whether the mouse is available at all on the
   current platform.

### Portability
                             X/Open  ncurses  NetBSD
    mouse_set                   -       -       -
    mouse_on                    -       -       -
    mouse_off                   -       -       -
    request_mouse_pos           -       -       -
    wmouse_position             -       -       -
    getmouse                    -       *       -
    mouseinterval               -       Y       -
    wenclose                    -       Y       -
    wmouse_trafo                -       Y       -
    mouse_trafo                 -       Y       -
    mousemask                   -       Y       -
    nc_getmouse                 -       *       -
    ungetmouse                  -       Y       -
    has_mouse                   -       Y       -

    * See above, under Description

**man-end****************************************************************/

#include <string.h>

static bool ungot = FALSE;

int mouse_set(mmask_t mbe)
{
    PDC_LOG(("mouse_set() - called: event %x\n", mbe));

    if (!SP)
        return ERR;

    SP->_trap_mbe = mbe;
    return PDC_mouse_set();
}

int mouse_on(mmask_t mbe)
{
    PDC_LOG(("mouse_on() - called: event %x\n", mbe));

    if (!SP)
        return ERR;

    SP->_trap_mbe |= mbe;
    return PDC_mouse_set();
}

int mouse_off(mmask_t mbe)
{
    PDC_LOG(("mouse_off() - called: event %x\n", mbe));

    if (!SP)
        return ERR;

    SP->_trap_mbe &= ~mbe;
    return PDC_mouse_set();
}

int request_mouse_pos(void)
{
    PDC_LOG(("request_mouse_pos() - called\n"));

    Mouse_status = SP->mouse_status;

    return OK;
}

void wmouse_position(WINDOW *win, int *y, int *x)
{
    PDC_LOG(("wmouse_position() - called\n"));

    if (win && wenclose(win, MOUSE_Y_POS, MOUSE_X_POS))
    {
        if (y)
            *y = MOUSE_Y_POS - win->_begy;
        if (x)
            *x = MOUSE_X_POS - win->_begx;
    }
    else
    {
        if (y)
            *y = -1;
        if (x)
            *x = -1;
    }
}

mmask_t getmouse(void)
{
    PDC_LOG(("getmouse() - called\n"));

    return SP ? SP->_trap_mbe : (mmask_t)0;
}

/* ncurses mouse interface */

int mouseinterval(int wait)
{
    int old_wait;

    PDC_LOG(("mouseinterval() - called: %d\n", wait));

    if (!SP)
        return ERR;

    old_wait = SP->mouse_wait;

    if (wait >= 0 && wait <= 1000)
        SP->mouse_wait = wait;

    return old_wait;
}

bool wenclose(const WINDOW *win, int y, int x)
{
    PDC_LOG(("wenclose() - called: %p %d %d\n", win, y, x));

    return (win && y >= win->_begy && y < win->_begy + win->_maxy
                && x >= win->_begx && x < win->_begx + win->_maxx);
}

bool wmouse_trafo(const WINDOW *win, int *y, int *x, bool to_screen)
{
    int newy, newx;

    PDC_LOG(("wmouse_trafo() - called\n"));

    if (!win || !y || !x)
        return FALSE;

    newy = *y;
    newx = *x;

    if (to_screen)
    {
        newy += win->_begy;
        newx += win->_begx;

        if (!wenclose(win, newy, newx))
            return FALSE;
    }
    else
    {
        if (wenclose(win, newy, newx))
        {
            newy -= win->_begy;
            newx -= win->_begx;
        }
        else
            return FALSE;
    }

    *y = newy;
    *x = newx;

    return TRUE;
}

bool mouse_trafo(int *y, int *x, bool to_screen)
{
    PDC_LOG(("mouse_trafo() - called\n"));

    return wmouse_trafo(stdscr, y, x, to_screen);
}

mmask_t mousemask(mmask_t mask, mmask_t *oldmask)
{
    PDC_LOG(("mousemask() - called\n"));

    if (!SP)
        return (mmask_t)0;

    if (oldmask)
        *oldmask = SP->_trap_mbe;

    /* The ncurses interface doesn't work with our move events, so
       filter them here */

    mask &= ~(BUTTON1_MOVED | BUTTON2_MOVED | BUTTON3_MOVED);

    mouse_set(mask);

    return SP->_trap_mbe;
}

int nc_getmouse(MEVENT *event)
{
    int i;
    mmask_t bstate = 0;

    PDC_LOG(("nc_getmouse() - called\n"));

    if (!event || !SP)
        return ERR;

    ungot = FALSE;

    request_mouse_pos();

    event->id = 0;

    event->x = Mouse_status.x;
    event->y = Mouse_status.y;
    event->z = 0;

    for (i = 0; i < 3; i++)
    {
        if (Mouse_status.changes & (1 << i))
        {
            int shf = i * 5;
            short button = Mouse_status.button[i] & BUTTON_ACTION_MASK;

            if (button == BUTTON_RELEASED)
                bstate |= (BUTTON1_RELEASED << shf);
            else if (button == BUTTON_PRESSED)
                bstate |= (BUTTON1_PRESSED << shf);
            else if (button == BUTTON_CLICKED)
                bstate |= (BUTTON1_CLICKED << shf);
            else if (button == BUTTON_DOUBLE_CLICKED)
                bstate |= (BUTTON1_DOUBLE_CLICKED << shf);

            button = Mouse_status.button[i] & BUTTON_MODIFIER_MASK;

            if (button & PDC_BUTTON_SHIFT)
                bstate |= BUTTON_MODIFIER_SHIFT;
            if (button & PDC_BUTTON_CONTROL)
                bstate |= BUTTON_MODIFIER_CONTROL;
            if (button & PDC_BUTTON_ALT)
                bstate |= BUTTON_MODIFIER_ALT;
        }
    }

    if (MOUSE_WHEEL_UP)
        bstate |= BUTTON4_PRESSED;
    else if (MOUSE_WHEEL_DOWN)
        bstate |= BUTTON5_PRESSED;

    /* extra filter pass -- mainly for button modifiers */

    event->bstate = bstate & SP->_trap_mbe;

    return OK;
}

int ungetmouse(MEVENT *event)
{
    int i;
    mmask_t bstate;

    PDC_LOG(("ungetmouse() - called\n"));

    if (!event || ungot)
        return ERR;

    ungot = TRUE;

    SP->mouse_status.x = event->x;
    SP->mouse_status.y = event->y;

    SP->mouse_status.changes = 0;
    bstate = event->bstate;

    for (i = 0; i < 3; i++)
    {
        int shf = i * 5;
        short button = 0;

        if (bstate & ((BUTTON1_RELEASED | BUTTON1_PRESSED |
            BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED) << shf))
        {
            SP->mouse_status.changes |= 1 << i;

            if (bstate & (BUTTON1_PRESSED << shf))
                button = BUTTON_PRESSED;
            if (bstate & (BUTTON1_CLICKED << shf))
                button = BUTTON_CLICKED;
            if (bstate & (BUTTON1_DOUBLE_CLICKED << shf))
                button = BUTTON_DOUBLE_CLICKED;

            if (bstate & BUTTON_MODIFIER_SHIFT)
                button |= PDC_BUTTON_SHIFT;
            if (bstate & BUTTON_MODIFIER_CONTROL)
                button |= PDC_BUTTON_CONTROL;
            if (bstate & BUTTON_MODIFIER_ALT)
                button |= PDC_BUTTON_ALT;
        }

        SP->mouse_status.button[i] = button;
    }

    if (bstate & BUTTON4_PRESSED)
        SP->mouse_status.changes |= PDC_MOUSE_WHEEL_UP;
    else if (bstate & BUTTON5_PRESSED)
        SP->mouse_status.changes |= PDC_MOUSE_WHEEL_DOWN;

    return PDC_ungetch(KEY_MOUSE);
}

bool has_mouse(void)
{
    return PDC_has_mouse();
}
