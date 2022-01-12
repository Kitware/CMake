/*----------------------------------------------------------------------*
 *                         Panels for PDCurses                          *
 *----------------------------------------------------------------------*/

#ifndef __PDCURSES_PANEL_H__
#define __PDCURSES_PANEL_H__ 1

#include <curses.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct panelobs
{
    struct panelobs *above;
    struct panel *pan;
} PANELOBS;

typedef struct panel
{
    WINDOW *win;
    int wstarty;
    int wendy;
    int wstartx;
    int wendx;
    struct panel *below;
    struct panel *above;
    const void *user;
    struct panelobs *obscure;
} PANEL;

PDCEX  int     bottom_panel(PANEL *pan);
PDCEX  int     del_panel(PANEL *pan);
PDCEX  int     hide_panel(PANEL *pan);
PDCEX  int     move_panel(PANEL *pan, int starty, int startx);
PDCEX  PANEL  *new_panel(WINDOW *win);
PDCEX  PANEL  *panel_above(const PANEL *pan);
PDCEX  PANEL  *panel_below(const PANEL *pan);
PDCEX  int     panel_hidden(const PANEL *pan);
PDCEX  const void *panel_userptr(const PANEL *pan);
PDCEX  WINDOW *panel_window(const PANEL *pan);
PDCEX  int     replace_panel(PANEL *pan, WINDOW *win);
PDCEX  int     set_panel_userptr(PANEL *pan, const void *uptr);
PDCEX  int     show_panel(PANEL *pan);
PDCEX  int     top_panel(PANEL *pan);
PDCEX  void    update_panels(void);

#ifdef __cplusplus
}
#endif

#endif /* __PDCURSES_PANEL_H__ */
