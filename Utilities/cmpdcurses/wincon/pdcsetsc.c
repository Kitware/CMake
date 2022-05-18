/* PDCurses */

#include "pdcwin.h"

/*man-start**************************************************************

pdcsetsc
--------

### Synopsis

    int PDC_set_blink(bool blinkon);
    int PDC_set_bold(bool boldon);
    void PDC_set_title(const char *title);

### Description

   PDC_set_blink() toggles whether the A_BLINK attribute sets an actual
   blink mode (TRUE), or sets the background color to high intensity
   (FALSE). The default is platform-dependent (FALSE in most cases). It
   returns OK if it could set the state to match the given parameter,
   ERR otherwise.

   PDC_set_bold() toggles whether the A_BOLD attribute selects an actual
   bold font (TRUE), or sets the foreground color to high intensity
   (FALSE). It returns OK if it could set the state to match the given
   parameter, ERR otherwise.

   PDC_set_title() sets the title of the window in which the curses
   program is running. This function may not do anything on some
   platforms.

### Portability
                             X/Open  ncurses  NetBSD
    PDC_set_blink               -       -       -
    PDC_set_title               -       -       -

**man-end****************************************************************/

int PDC_curs_set(int visibility)
{
    CONSOLE_CURSOR_INFO cci;
    int ret_vis;

    PDC_LOG(("PDC_curs_set() - called: visibility=%d\n", visibility));

    ret_vis = SP->visibility;

    if (GetConsoleCursorInfo(pdc_con_out, &cci) == FALSE)
        return ERR;

    switch(visibility)
    {
    case 0:             /* invisible */
        cci.bVisible = FALSE;
        break;
    case 2:             /* highly visible */
        cci.bVisible = TRUE;
        cci.dwSize = 95;
        break;
    default:            /* normal visibility */
        cci.bVisible = TRUE;
        cci.dwSize = SP->orig_cursor;
        break;
    }

    if (SetConsoleCursorInfo(pdc_con_out, &cci) == FALSE)
        return ERR;

    SP->visibility = visibility;
    return ret_vis;
}

void PDC_set_title(const char *title)
{
#ifdef PDC_WIDE
    wchar_t wtitle[512];
#endif
    PDC_LOG(("PDC_set_title() - called:<%s>\n", title));

#ifdef PDC_WIDE
    PDC_mbstowcs(wtitle, title, 511);
    SetConsoleTitleW(wtitle);
#else
    SetConsoleTitleA(title);
#endif
}

int PDC_set_blink(bool blinkon)
{
    if (!SP)
        return ERR;

    if (SP->color_started)
    {
        COLORS = 16;
        if (PDC_can_change_color()) /* is_nt */
        {
            if (pdc_conemu || SetConsoleMode(pdc_con_out, 0x0004)) /* VT */
                COLORS = PDC_MAXCOL;

            if (!pdc_conemu)
                SetConsoleMode(pdc_con_out, 0x0010); /* LVB */
        }
    }

    if (blinkon)
    {
        if (!(SP->termattrs & A_BLINK))
        {
            SP->termattrs |= A_BLINK;
            pdc_last_blink = GetTickCount();
        }
    }
    else
    {
        if (SP->termattrs & A_BLINK)
        {
            SP->termattrs &= ~A_BLINK;
            PDC_blink_text();
        }
    }

    return OK;
}

int PDC_set_bold(bool boldon)
{
    return boldon ? ERR : OK;
}
