#
# This module finds if CABLE is installed and determines where the
# include files and libraries are.  This code sets the following variables:
#
#  CABLE              = the full path to the cable executable
#  CABLE_TCL_LIBRARY  = the full path to the Tcl wrapper facility library
#  CABLE_INCLUDE_DIR  = the full path to the cable include directory
#
#  To build Tcl wrappers, you should add shared library and link it to
#  ${CABLE_TCL_LIBRARY}.  You should also add ${CABLE_INCLUDE_DIR} as
#  an include directory.
#

IF(NOT CABLE)
  FIND_PATH(CABLE_BUILD_DIR cable.README.txt)
ENDIF(NOT CABLE)

IF(CABLE_BUILD_DIR)
  LOAD_CACHE(${CABLE_BUILD_DIR}
             EXCLUDE
               BUILD_SHARED_LIBS
               LIBRARY_OUTPUT_PATH
               EXECUTABLE_OUTPUT_PATH
               MAKECOMMAND
             INCLUDE_INTERNALS
               CABLE_LIBRARY_OUTPUT_PATH
               CABLE_EXECUTABLE_OUTPUT_PATH)
  
  IF(CABLE_LIBRARY_OUTPUT_PATH)
    FIND_LIBRARY(CABLE_TCL_LIBRARY NAMES WrapTclFacility PATHS
                 ${CABLE_LIBRARY_OUTPUT_PATH})
  ELSE(CABLE_LIBRARY_OUTPUT_PATH)
    FIND_LIBRARY(CABLE_TCL_LIBRARY NAMES WrapTclFacility PATHS
                 ${CABLE_BINARY_DIR}/WrapTclFacility)
  ENDIF(CABLE_LIBRARY_OUTPUT_PATH)
  
  IF(CABLE_EXECUTABLE_OUTPUT_PATH)
    FIND_PROGRAM(CABLE NAMES cable PATHS ${CABLE_EXECUTABLE_OUTPUT_PATH})
  ELSE(CABLE_EXECUTABLE_OUTPUT_PATH)
    FIND_PROGRAM(CABLE NAMES cable PATHS ${CABLE_BINARY_DIR}/Executables)
  ENDIF(CABLE_EXECUTABLE_OUTPUT_PATH)
   
  SET(CABLE_INCLUDE_DIR ${CABLE_SOURCE_DIR}
      CACHE STRING "Include path for Cable headers.")
ELSE(CABLE_BUILD_DIR)
  # Use some tricks to find the cable executable relative to somewhere
  # in the system search path.  CMake will collapse the relative paths
  # automatically.
  FIND_PROGRAM(CABLE
    NAMES cable
          ../share/Cable/bin/cable
          ../Cable/bin/cable
    PATHS /usr/share/Cable/bin
          /usr/local/share/Cable/bin
          "C:/Program Files/Cable/bin"
  )

  # Get the path where the executable sits, but without the executable
  # name on it.
  GET_FILENAME_COMPONENT(CABLE_ROOT_BIN ${CABLE} PATH)
  
  # Find the cable include directory in a path relative to the cable
  # executable.
  FIND_PATH(CABLE_INCLUDE_DIR WrapTclFacility/wrapCalls.h
            ${CABLE_ROOT_BIN}/../include)
  
  # Find the WrapTclFacility library in a path relative to the cable
  # executable.
  FIND_LIBRARY(CABLE_TCL_LIBRARY NAMES WrapTclFacility PATHS
               ${CABLE_ROOT_BIN}/../lib)  
ENDIF(CABLE_BUILD_DIR)
