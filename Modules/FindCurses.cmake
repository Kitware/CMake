#
# Find the curses include file and library
#

FIND_PATH(CURSES_INCLUDE_PATH curses.h
/usr/local/include /usr/include
)

FIND_LIBRARY(CURSES_LIBRARY curses
PATHS /usr/local/lib /usr/lib 
)

FIND_LIBRARY(CURSES_EXTRA_LIBRARY cur_colr
PATHS /usr/local/lib /usr/lib 
)

FIND_LIBRARY(FORM_LIBRARY form
PATHS /usr/local/lib /usr/lib 
)

