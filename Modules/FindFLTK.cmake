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
  SET ( FLTK_WRAP_UI 1 CACHE INTERNAL "Can we honour the FLTK_WRAP_UI command" )
ENDIF (FLTK_FLUID_EXE)


#
#  Set HAS_FLTK
#  This is the final flag that will be checked by
#  other code that requires FLTK for compile/run.
#
IF(FLTK_FLUID_EXE)
  IF(FLTK_INCLUDE_PATH)
    IF(FLTK_LIBRARY)
      SET (HAS_FLTK 1 CACHE INTERNAL "FLTK library, headers and Fluid are available")
    ENDIF(FLTK_LIBRARY)
  ENDIF(FLTK_INCLUDE_PATH)
ENDIF(FLTK_FLUID_EXE)


