/* Private definitions and declarations for use within PDCurses.
   These should generally not be referenced by applications. */

#ifndef __CURSES_INTERNALS__
#define __CURSES_INTERNALS__ 1

#define CURSES_LIBRARY
#include <curses.h>

#if defined(__TURBOC__) || defined(__EMX__) || defined(__DJGPP__) || \
    defined(PDC_99) || defined(__WATCOMC__)
# ifndef HAVE_VSSCANF
#  define HAVE_VSSCANF       /* have vsscanf() */
# endif
#endif

#if defined(PDC_99) || defined(__WATCOMC__)
# ifndef HAVE_VSNPRINTF
#  define HAVE_VSNPRINTF     /* have vsnprintf() */
# endif
#endif

/*----------------------------------------------------------------------*/

typedef struct           /* structure for ripped off lines */
{
    int line;
    int (*init)(WINDOW *, int);
} RIPPEDOFFLINE;

/* Window properties */

#define _SUBWIN    0x01  /* window is a subwindow */
#define _PAD       0x10  /* X/Open Pad. */
#define _SUBPAD    0x20  /* X/Open subpad. */

/* Miscellaneous */

#define _NO_CHANGE -1    /* flags line edge unchanged */

#define _ECHAR     0x08  /* Erase char       (^H) */
#define _DWCHAR    0x17  /* Delete Word char (^W) */
#define _DLCHAR    0x15  /* Delete Line char (^U) */

/*----------------------------------------------------------------------*/

/* Platform implementation functions */

void    PDC_beep(void);
bool    PDC_can_change_color(void);
int     PDC_color_content(short, short *, short *, short *);
bool    PDC_check_key(void);
int     PDC_curs_set(int);
void    PDC_doupdate(void);
void    PDC_flushinp(void);
int     PDC_get_columns(void);
int     PDC_get_cursor_mode(void);
int     PDC_get_key(void);
int     PDC_get_rows(void);
void    PDC_gotoyx(int, int);
bool    PDC_has_mouse(void);
int     PDC_init_color(short, short, short, short);
int     PDC_modifiers_set(void);
int     PDC_mouse_set(void);
void    PDC_napms(int);
void    PDC_reset_prog_mode(void);
void    PDC_reset_shell_mode(void);
int     PDC_resize_screen(int, int);
void    PDC_restore_screen_mode(int);
void    PDC_save_screen_mode(int);
#ifdef XCURSES
void    PDC_set_args(int, char **);
#endif
void    PDC_scr_close(void);
void    PDC_scr_free(void);
int     PDC_scr_open(void);
void    PDC_set_keyboard_binary(bool);
void    PDC_transform_line(int, int, int, const chtype *);
const char *PDC_sysname(void);

/* Internal cross-module functions */

void    PDC_init_atrtab(void);
WINDOW *PDC_makelines(WINDOW *);
WINDOW *PDC_makenew(int, int, int, int);
int     PDC_mouse_in_slk(int, int);
void    PDC_slk_free(void);
void    PDC_slk_initialize(void);
void    PDC_sync(WINDOW *);

#ifdef PDC_WIDE
int     PDC_mbtowc(wchar_t *, const char *, size_t);
size_t  PDC_mbstowcs(wchar_t *, const char *, size_t);
size_t  PDC_wcstombs(char *, const wchar_t *, size_t);
#endif

#ifdef PDCDEBUG
# define PDC_LOG(x) if (SP && SP->dbfp) PDC_debug x
#else
# define PDC_LOG(x)
#endif

/* Internal macros for attributes */

#ifndef max
# define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define DIVROUND(num, divisor) ((num) + ((divisor) >> 1)) / (divisor)

#define PDC_CLICK_PERIOD 150  /* time to wait for a click, if
                                 not set by mouseinterval() */
#define PDC_COLOR_PAIRS  256
#define PDC_MAXCOL       768  /* maximum possible COLORS; may be less */

#define _INBUFSIZ        512  /* size of terminal input buffer */
#define NUNGETCH         256  /* max # chars to ungetch() */

#endif /* __CURSES_INTERNALS__ */
