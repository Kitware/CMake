/* PDCurses */

#include <curspriv.h>

/*man-start**************************************************************

color
-----

### Synopsis

    bool has_colors(void);
    int start_color(void);
    int init_pair(short pair, short fg, short bg);
    int pair_content(short pair, short *fg, short *bg);
    bool can_change_color(void);
    int init_color(short color, short red, short green, short blue);
    int color_content(short color, short *red, short *green, short *blue);

    int alloc_pair(int fg, int bg);
    int assume_default_colors(int f, int b);
    int find_pair(int fg, int bg);
    int free_pair(int pair);
    int use_default_colors(void);

    int PDC_set_line_color(short color);

### Description

   To use these routines, first, call start_color(). Colors are always
   used in pairs, referred to as color-pairs. A color-pair is created by
   init_pair(), and consists of a foreground color and a background
   color. After initialization, COLOR_PAIR(n) can be used like any other
   video attribute.

   has_colors() reports whether the terminal supports color.

   start_color() initializes eight basic colors (black, red, green,
   yellow, blue, magenta, cyan, and white), and two global variables:
   COLORS and COLOR_PAIRS (respectively defining the maximum number of
   colors and color-pairs the terminal is capable of displaying).

   init_pair() changes the definition of a color-pair. It takes three
   arguments: the number of the color-pair to be redefined, and the new
   values of the foreground and background colors. The pair number must
   be between 0 and COLOR_PAIRS - 1, inclusive. The foreground and
   background must be between 0 and COLORS - 1, inclusive. If the color
   pair was previously initialized, the screen is refreshed, and all
   occurrences of that color-pair are changed to the new definition.

   pair_content() is used to determine what the colors of a given color-
   pair consist of.

   can_change_color() indicates if the terminal has the capability to
   change the definition of its colors.

   init_color() is used to redefine a color, if possible. Each of the
   components -- red, green, and blue -- is specified in a range from 0
   to 1000, inclusive.

   color_content() reports the current definition of a color in the same
   format as used by init_color().

   assume_default_colors() and use_default_colors() emulate the ncurses
   extensions of the same names. assume_default_colors(f, b) is
   essentially the same as init_pair(0, f, b) (which isn't allowed); it
   redefines the default colors. use_default_colors() allows the use of
   -1 as a foreground or background color with init_pair(), and calls
   assume_default_colors(-1, -1); -1 represents the foreground or
   background color that the terminal had at startup. If the environment
   variable PDC_ORIGINAL_COLORS is set at the time start_color() is
   called, that's equivalent to calling use_default_colors().

   alloc_pair(), find_pair() and free_pair() are also from ncurses.
   free_pair() marks a pair as unused; find_pair() returns an existing
   pair with the specified foreground and background colors, if one
   exists. And alloc_pair() returns such a pair whether or not it was
   previously set, overwriting the oldest initialized pair if there are
   no free pairs.

   PDC_set_line_color() is used to set the color, globally, for the
   color of the lines drawn for the attributes: A_UNDERLINE, A_LEFT and
   A_RIGHT. A value of -1 (the default) indicates that the current
   foreground color should be used.

   NOTE: COLOR_PAIR() and PAIR_NUMBER() are implemented as macros.

### Return Value

   Most functions return OK on success and ERR on error. has_colors()
   and can_change_colors() return TRUE or FALSE. alloc_pair() and
   find_pair() return a pair number, or -1 on error.

### Portability
                             X/Open  ncurses  NetBSD
    has_colors                  Y       Y       Y
    start_color                 Y       Y       Y
    init_pair                   Y       Y       Y
    pair_content                Y       Y       Y
    can_change_color            Y       Y       Y
    init_color                  Y       Y       Y
    color_content               Y       Y       Y
    alloc_pair                  -       Y       -
    assume_default_colors       -       Y       Y
    find_pair                   -       Y       -
    free_pair                   -       Y       -
    use_default_colors          -       Y       Y
    PDC_set_line_color          -       -       -

**man-end****************************************************************/

#include <stdlib.h>
#include <string.h>

int COLORS = 0;
int COLOR_PAIRS = PDC_COLOR_PAIRS;

static bool default_colors = FALSE;
static short first_col = 0;
static int allocnum = 0;

int start_color(void)
{
    PDC_LOG(("start_color() - called\n"));

    if (!SP || SP->mono)
        return ERR;

    SP->color_started = TRUE;

    PDC_set_blink(FALSE);   /* Also sets COLORS */

    if (!default_colors && SP->orig_attr && getenv("PDC_ORIGINAL_COLORS"))
        default_colors = TRUE;

    PDC_init_atrtab();

    return OK;
}

static void _normalize(short *fg, short *bg)
{
    if (*fg == -1)
        *fg = SP->orig_attr ? SP->orig_fore : COLOR_WHITE;

    if (*bg == -1)
        *bg = SP->orig_attr ? SP->orig_back : COLOR_BLACK;
}

static void _init_pair_core(short pair, short fg, short bg)
{
    PDC_PAIR *p = SP->atrtab + pair;

    _normalize(&fg, &bg);

    /* To allow the PDC_PRESERVE_SCREEN option to work, we only reset
       curscr if this call to init_pair() alters a color pair created by
       the user. */

    if (p->set)
    {
        if (p->f != fg || p->b != bg)
            curscr->_clear = TRUE;
    }

    p->f = fg;
    p->b = bg;
    p->count = allocnum++;
    p->set = TRUE;
}

int init_pair(short pair, short fg, short bg)
{
    PDC_LOG(("init_pair() - called: pair %d fg %d bg %d\n", pair, fg, bg));

    if (!SP || !SP->color_started || pair < 1 || pair >= COLOR_PAIRS ||
        fg < first_col || fg >= COLORS || bg < first_col || bg >= COLORS)
        return ERR;

    _init_pair_core(pair, fg, bg);

    return OK;
}

bool has_colors(void)
{
    PDC_LOG(("has_colors() - called\n"));

    return SP ? !(SP->mono) : FALSE;
}

int init_color(short color, short red, short green, short blue)
{
    PDC_LOG(("init_color() - called\n"));

    if (!SP || color < 0 || color >= COLORS || !PDC_can_change_color() ||
        red < -1 || red > 1000 || green < -1 || green > 1000 ||
        blue < -1 || blue > 1000)
        return ERR;

    SP->dirty = TRUE;

    return PDC_init_color(color, red, green, blue);
}

int color_content(short color, short *red, short *green, short *blue)
{
    PDC_LOG(("color_content() - called\n"));

    if (color < 0 || color >= COLORS || !red || !green || !blue)
        return ERR;

    if (PDC_can_change_color())
        return PDC_color_content(color, red, green, blue);
    else
    {
        /* Simulated values for platforms that don't support palette
           changing */

        short maxval = (color & 8) ? 1000 : 680;

        *red = (color & COLOR_RED) ? maxval : 0;
        *green = (color & COLOR_GREEN) ? maxval : 0;
        *blue = (color & COLOR_BLUE) ? maxval : 0;

        return OK;
    }
}

bool can_change_color(void)
{
    PDC_LOG(("can_change_color() - called\n"));

    return PDC_can_change_color();
}

int pair_content(short pair, short *fg, short *bg)
{
    PDC_LOG(("pair_content() - called\n"));

    if (pair < 0 || pair >= COLOR_PAIRS || !fg || !bg)
        return ERR;

    *fg = SP->atrtab[pair].f;
    *bg = SP->atrtab[pair].b;

    return OK;
}

int assume_default_colors(int f, int b)
{
    PDC_LOG(("assume_default_colors() - called: f %d b %d\n", f, b));

    if (f < -1 || f >= COLORS || b < -1 || b >= COLORS)
        return ERR;

    if (SP->color_started)
        _init_pair_core(0, f, b);

    return OK;
}

int use_default_colors(void)
{
    PDC_LOG(("use_default_colors() - called\n"));

    default_colors = TRUE;
    first_col = -1;

    return assume_default_colors(-1, -1);
}

int PDC_set_line_color(short color)
{
    PDC_LOG(("PDC_set_line_color() - called: %d\n", color));

    if (!SP || color < -1 || color >= COLORS)
        return ERR;

    SP->line_color = color;

    return OK;
}

void PDC_init_atrtab(void)
{
    PDC_PAIR *p = SP->atrtab;
    short i, fg, bg;

    if (SP->color_started && !default_colors)
    {
        fg = COLOR_WHITE;
        bg = COLOR_BLACK;
    }
    else
        fg = bg = -1;

    _normalize(&fg, &bg);

    for (i = 0; i < PDC_COLOR_PAIRS; i++)
    {
        p[i].f = fg;
        p[i].b = bg;
        p[i].set = FALSE;
    }
}

int free_pair(int pair)
{
    if (pair < 1 || pair >= PDC_COLOR_PAIRS || !(SP->atrtab[pair].set))
        return ERR;

    SP->atrtab[pair].set = FALSE;
    return OK;
}

int find_pair(int fg, int bg)
{
    int i;
    PDC_PAIR *p = SP->atrtab;

    for (i = 0; i < PDC_COLOR_PAIRS; i++)
        if (p[i].set && p[i].f == fg && p[i].b == bg)
            return i;

    return -1;
}

static int _find_oldest()
{
    int i, lowind = 0, lowval = 0;
    PDC_PAIR *p = SP->atrtab;

    for (i = 1; i < PDC_COLOR_PAIRS; i++)
    {
        if (!p[i].set)
            return i;

        if (!lowval || (p[i].count < lowval))
        {
            lowind = i;
            lowval = p[i].count;
        }
    }

    return lowind;
}

int alloc_pair(int fg, int bg)
{
    int i = find_pair(fg, bg);

    if (-1 == i)
    {
        i = _find_oldest();

        if (ERR == init_pair(i, fg, bg))
            return -1;
    }

    return i;
}
