#
# Find the native FLTK includes and library
#


FIND_PATH(FLTK_INCLUDE_PATH FL/Fl.h
/usr/local/include
/usr/include
/usr/local/fltk
/usr/X11R6/include
H:/usr/local/fltk
)

FIND_LIBRARY(FLTK_LIBRARY  fltk
PATHS /usr/lib /usr/local/lib /usr/local/fltk/lib H:/usr/local/fltk/lib /usr/X11R6/lib
)

