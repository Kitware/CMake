#
# Find the native FLTK includes and library
#
# FLTK_FLUID_EXE, where to find the Fluid tool
# FLTK_WRAP_UI, This allows the FLTK_WRAP_UI command to work.


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

FIND_FILE(FLTK_FLUID_EXE fluid
${path}
)

IF (FLTK_FLUID_EXE)
  SET ( FLTK_WRAP_UI 1 CACHE BOOL "Can we honour the FLTK_WRAP_UI command" )
ENDIF (FLTK_FLUID_EXE)

MARK_AS_ADVANCED(
  FLTK_INCLUDE_PATH
  FLTK_LIBRARY
  FLTK_FLUID_EXE
)