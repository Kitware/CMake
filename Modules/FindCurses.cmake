#
# Find the curses include file and library
#

FIND_PATH(CURSES_INCLUDE_PATH curses.h
/usr/local/include
/usr/include
)

FIND_LIBRARY(CURSES_LIBRARY curses
PATHS /usr/lib /usr/local/lib
)

FIND_LIBRARY(FORM_LIBRARY form
PATHS /usr/lib /usr/local/lib
)
