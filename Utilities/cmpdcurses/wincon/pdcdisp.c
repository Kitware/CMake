/* PDCurses */

#include "pdcwin.h"

#include <stdlib.h>
#include <string.h>

#ifdef PDC_WIDE
# include "../common/acsuni.h"
#else
# include "../common/acs437.h"
#endif

DWORD pdc_last_blink;
static bool blinked_off = FALSE;
static bool in_italic = FALSE;

/* position hardware cursor at (y, x) */

void PDC_gotoyx(int row, int col)
{
    COORD coord;

    PDC_LOG(("PDC_gotoyx() - called: row %d col %d from row %d col %d\n",
             row, col, SP->cursrow, SP->curscol));

    coord.X = col;
    coord.Y = row;

    SetConsoleCursorPosition(pdc_con_out, coord);
}

void _set_ansi_color(short f, short b, attr_t attr)
{
    char esc[64], *p;
    short tmp, underline;
    bool italic;

    if (f < 16 && !pdc_color[f].mapped)
        f = pdc_curstoansi[f];

    if (b < 16 && !pdc_color[b].mapped)
        b = pdc_curstoansi[b];

    if (attr & A_REVERSE)
    {
        tmp = f;
        f = b;
        b = tmp;
    }
    attr &= SP->termattrs;
    italic = !!(attr & A_ITALIC);
    underline = !!(attr & A_UNDERLINE);

    p = esc + sprintf(esc, "\x1b[");

    if (f != pdc_oldf)
    {
        if (f < 8 && !pdc_color[f].mapped)
            p += sprintf(p, "%d", f + 30);
        else if (f < 16 && !pdc_color[f].mapped)
            p += sprintf(p, "%d", f + 82);
        else if (f < 256 && !pdc_color[f].mapped)
            p += sprintf(p, "38;5;%d", f);
        else
        {
            short red = DIVROUND(pdc_color[f].r * 255, 1000);
            short green = DIVROUND(pdc_color[f].g * 255, 1000);
            short blue = DIVROUND(pdc_color[f].b * 255, 1000);

            p += sprintf(p, "38;2;%d;%d;%d", red, green, blue);
        }

        pdc_oldf = f;
    }

    if (b != pdc_oldb)
    {
        if (strlen(esc) > 2)
            p += sprintf(p, ";");

        if (b < 8 && !pdc_color[b].mapped)
            p += sprintf(p, "%d", b + 40);
        else if (b < 16 && !pdc_color[b].mapped)
            p += sprintf(p, "%d", b + 92);
        else if (b < 256 && !pdc_color[b].mapped)
            p += sprintf(p, "48;5;%d", b);
        else
        {
            short red = DIVROUND(pdc_color[b].r * 255, 1000);
            short green = DIVROUND(pdc_color[b].g * 255, 1000);
            short blue = DIVROUND(pdc_color[b].b * 255, 1000);

            p += sprintf(p, "48;2;%d;%d;%d", red, green, blue);
        }

        pdc_oldb = b;
    }

    if (italic != in_italic)
    {
        if (strlen(esc) > 2)
            p += sprintf(p, ";");

        if (italic)
            p += sprintf(p, "3");
        else
            p += sprintf(p, "23");

        in_italic = italic;
    }

    if (underline != pdc_oldu)
    {
        if (strlen(esc) > 2)
            p += sprintf(p, ";");

        if (underline)
            p += sprintf(p, "4");
        else
            p += sprintf(p, "24");

        pdc_oldu = underline;
    }

    if (strlen(esc) > 2)
    {
        sprintf(p, "m");
        if (!pdc_conemu)
            SetConsoleMode(pdc_con_out, 0x0015);

        WriteConsoleA(pdc_con_out, esc, strlen(esc), NULL, NULL);

        if (!pdc_conemu)
            SetConsoleMode(pdc_con_out, 0x0010);
    }
}

void _new_packet(attr_t attr, int lineno, int x, int len, const chtype *srcp)
{
    int j;
    short fore, back;
    bool blink, ansi;

    if (pdc_ansi && (lineno == (SP->lines - 1)) && ((x + len) == SP->cols))
    {
        len--;
        if (len)
            _new_packet(attr, lineno, x, len, srcp);
        pdc_ansi = FALSE;
        _new_packet(attr, lineno, x + len, 1, srcp + len);
        pdc_ansi = TRUE;
        return;
    }

    pair_content(PAIR_NUMBER(attr), &fore, &back);
    ansi = pdc_ansi || (fore >= 16 || back >= 16);
    blink = (SP->termattrs & A_BLINK) && (attr & A_BLINK);

    if (blink)
    {
        attr &= ~A_BLINK;
        if (blinked_off)
            attr &= ~(A_UNDERLINE | A_RIGHT | A_LEFT);
    }

    if (attr & A_BOLD)
        fore |= 8;
    if (attr & A_BLINK)
        back |= 8;

    if (ansi)
    {
#ifdef PDC_WIDE
        WCHAR buffer[512];
#else
        char buffer[512];
#endif
        for (j = 0; j < len; j++)
        {
            chtype ch = srcp[j];

            if (ch & A_ALTCHARSET && !(ch & 0xff80))
            {
                ch = acs_map[ch & 0x7f];

                if (pdc_wt && (ch & A_CHARTEXT) < ' ')
                    goto NONANSI;
            }

            if (blink && blinked_off)
                ch = ' ';

            buffer[j] = ch & A_CHARTEXT;
        }

        PDC_gotoyx(lineno, x);
        _set_ansi_color(fore, back, attr);
#ifdef PDC_WIDE
        WriteConsoleW(pdc_con_out, buffer, len, NULL, NULL);
#else
        WriteConsoleA(pdc_con_out, buffer, len, NULL, NULL);
#endif
    }
    else
NONANSI:
    {
        CHAR_INFO buffer[512];
        COORD bufSize, bufPos;
        SMALL_RECT sr;
        WORD mapped_attr;

        fore = pdc_curstoreal[fore];
        back = pdc_curstoreal[back];

        if (attr & A_REVERSE)
            mapped_attr = back | (fore << 4);
        else
            mapped_attr = fore | (back << 4);

        if (attr & A_UNDERLINE)
            mapped_attr |= 0x8000; /* COMMON_LVB_UNDERSCORE */
        if (attr & A_LEFT)
            mapped_attr |= 0x0800; /* COMMON_LVB_GRID_LVERTICAL */
        if (attr & A_RIGHT)
            mapped_attr |= 0x1000; /* COMMON_LVB_GRID_RVERTICAL */

        for (j = 0; j < len; j++)
        {
            chtype ch = srcp[j];

            if (ch & A_ALTCHARSET && !(ch & 0xff80))
                ch = acs_map[ch & 0x7f];

            if (blink && blinked_off)
                ch = ' ';

            buffer[j].Attributes = mapped_attr;
            buffer[j].Char.UnicodeChar = ch & A_CHARTEXT;
        }

        bufPos.X = bufPos.Y = 0;
        bufSize.X = len;
        bufSize.Y = 1;

        sr.Top = sr.Bottom = lineno;
        sr.Left = x;
        sr.Right = x + len - 1;

        WriteConsoleOutput(pdc_con_out, buffer, bufSize, bufPos, &sr);
    }
}

/* update the given physical line to look like the corresponding line in
   curscr */

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    attr_t old_attr, attr;
    int i, j;

    PDC_LOG(("PDC_transform_line() - called: lineno=%d\n", lineno));

    old_attr = *srcp & (A_ATTRIBUTES ^ A_ALTCHARSET);

    for (i = 1, j = 1; j < len; i++, j++)
    {
        attr = srcp[i] & (A_ATTRIBUTES ^ A_ALTCHARSET);

        if (attr != old_attr)
        {
            _new_packet(old_attr, lineno, x, i, srcp);
            old_attr = attr;
            srcp += i;
            x += i;
            i = 0;
        }
    }

    _new_packet(old_attr, lineno, x, i, srcp);
}

void PDC_blink_text(void)
{
    CONSOLE_CURSOR_INFO cci;
    int i, j, k;
    bool oldvis;

    GetConsoleCursorInfo(pdc_con_out, &cci);
    oldvis = cci.bVisible;
    if (oldvis)
    {
        cci.bVisible = FALSE;
        SetConsoleCursorInfo(pdc_con_out, &cci);
    }

    if (!(SP->termattrs & A_BLINK))
        blinked_off = FALSE;
    else
        blinked_off = !blinked_off;

    for (i = 0; i < SP->lines; i++)
    {
        const chtype *srcp = curscr->_y[i];

        for (j = 0; j < SP->cols; j++)
            if (srcp[j] & A_BLINK)
            {
                k = j;
                while (k < SP->cols && (srcp[k] & A_BLINK))
                    k++;
                PDC_transform_line(i, j, k - j, srcp + j);
                j = k;
            }
    }

    PDC_gotoyx(SP->cursrow, SP->curscol);
    if (oldvis)
    {
        cci.bVisible = TRUE;
        SetConsoleCursorInfo(pdc_con_out, &cci);
    }

    pdc_last_blink = GetTickCount();
}

void PDC_doupdate(void)
{
}
