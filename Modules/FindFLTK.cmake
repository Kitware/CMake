#
# Find the native FLTK includes and library
#
# FLTK_FLUID_EXE, where to find the Fluid tool
# FLTK_WRAP_UI, This allows the FLTK_WRAP_UI command to work.
# FLTK_INCLUDE_PATH, where to find include files
# FLTK_LIBRARY, list of fltk libraries
# FLTK_BASE_LIBRARY, the full path to fltk.lib
# FLTK_GL_LIBRARY, the full path to fltk_gl.lib
# FLTK_FORMS_LIBRARY, the full path to fltk_forms.lib


OPTION(USE_FLTK_VERSION_1.0.11 "Use FLTK version 1.0.11" 1)

OPTION(USE_FLTK_VERSION_1.1 "Use FLTK version 1.1" 0)

# Exlusion between the two version
IF(USE_FLTK_VERSION_1.0.11)
  SET(USE_FLTK_VERSION_1.1 0)
ENDIF(USE_FLTK_VERSION_1.0.11)


FIND_PATH(FLTK_INCLUDE_PATH FL/Fl.h
/usr/local/include
/usr/include
/usr/local/fltk
/usr/X11R6/include
H:/usr/local/fltk
)

# Make sure that the FLTK include path has been set
# So the FLTK_LIBRARY does not appear the first time
IF(FLTK_INCLUDE_PATH)
  IF(USE_FLTK_VERSION_1.0.11)
    FIND_LIBRARY(FLTK_BASE_LIBRARY  fltk
     PATHS /usr/lib /usr/local/lib /usr/local/fltk/lib H:/usr/local/fltk/lib /usr/X11R6/lib
       ${FLTK_INCLUDE_PATH}/lib
    )
  ENDIF(USE_FLTK_VERSION_1.0.11)


  IF(USE_FLTK_VERSION_1.1)
    FIND_LIBRARY(FLTK_BASE_LIBRARY  fltk
     PATHS /usr/lib /usr/local/lib /usr/local/fltk/lib H:/usr/local/fltk/lib /usr/X11R6/lib
           ${FLTK_INCLUDE_PATH}/lib
    )
   FIND_LIBRARY(FLTK_GL_LIBRARY NAMES fltkgl fltk_gl
     PATHS /usr/lib /usr/local/lib /usr/local/fltk/lib H:/usr/local/fltk/lib /usr/X11R6/lib
           ${FLTK_INCLUDE_PATH}/lib
    )
   FIND_LIBRARY(FLTK_FORMS_LIBRARY NAMES fltkforms fltk_forms
     PATHS /usr/lib /usr/local/lib /usr/local/fltk/lib H:/usr/local/fltk/lib /usr/X11R6/lib
           ${FLTK_INCLUDE_PATH}/lib
    )
  ENDIF(USE_FLTK_VERSION_1.1)

  SET( FLTK_LIBRARY ${FLTK_BASE_LIBRARY} ${FLTK_GL_LIBRARY} ${FLTK_FORMS_LIBRARY} )

ENDIF(FLTK_INCLUDE_PATH)

# Find Fluid
FIND_FILE(FLTK_FLUID_EXE fluid
${path} ${FLTK_INCLUDE_PATH}/fluid
)

# Enable the Wrap UI command
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


