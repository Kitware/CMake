#
# Find the native FLTK includes and library
#


FIND_PATH(FLTK_INCLUDE_PATH FL/FL.H
/usr/local/include
/usr/include
/usr/local/fltk
H:/usr/local/fltk
)

FIND_LIBRARY(FLTK_LIB_PATH fltk
/usr/lib
/usr/local/lib
/usr/local/fltk/lib
H:/usr/local/fltk/lib
)

