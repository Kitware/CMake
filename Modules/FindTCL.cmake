#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TCL_LIB_PATH     = the path to where the TCL library is
#  TCL_LIBRARY      = the name of the tcl library found (tcl tcl80 etc)
#  TCL_INCLUDE_PATH = the path to where tcl.h can be found
#
# 

# try to find the Tcl libraries in a few places and names
IF (NOT TCL_LIB_PATH)
  FIND_LIBRARY(TCL_LIB_PATH tcl "C:/Program Files/Tcl/lib" /usr/lib /usr/local/lib)
  IF (TCL_LIB_PATH)
    SET (TCL_LIBRARY tcl CACHE)
  ENDIF (TCL_LIB_PATH)
ENDIF (NOT TCL_LIB_PATH)

IF (NOT TCL_LIB_PATH)
  FIND_LIBRARY(TCL_LIB_PATH tcl82 "C:/Program Files/Tcl/lib" /usr/lib /usr/local/lib)
  IF (TCL_LIB_PATH)
    SET (TCL_LIBRARY tcl82 CACHE)
  ENDIF (TCL_LIB_PATH)
ENDIF (NOT TCL_LIB_PATH)

IF (NOT TCL_LIB_PATH)
  FIND_LIBRARY(TCL_LIB_PATH tcl80 "C:/Program Files/Tcl/lib" /usr/lib /usr/local/lib)
  IF (TCL_LIB_PATH)
    SET (TCL_LIBRARY tcl80 CACHE)
  ENDIF (TCL_LIB_PATH)
ENDIF (NOT TCL_LIB_PATH)

# add in the include path    
FIND_PATH(TCL_INCLUDE_PATH tcl.h /usr/include /usr/local/include)
