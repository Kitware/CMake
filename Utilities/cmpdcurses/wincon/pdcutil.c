/* PDCurses */

#include "pdcwin.h"

void PDC_beep(void)
{
    PDC_LOG(("PDC_beep() - called\n"));

/*  MessageBeep(MB_OK); */
    MessageBeep(0XFFFFFFFF);
}

void PDC_napms(int ms)
{
    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    if ((SP->termattrs & A_BLINK) && (GetTickCount() >= pdc_last_blink + 500))
        PDC_blink_text();

    Sleep(ms);
}

const char *PDC_sysname(void)
{
    return "Windows";
}
