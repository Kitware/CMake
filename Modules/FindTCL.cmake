#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TCL_LIB_PATH     = the path to where the TCL library is
#  TCL_LIBRARY      = the name of the tcl library found (tcl tcl80 etc)
#  TCL_INCLUDE_PATH = the path to where tcl.h can be found
#  TK_LIB_PATH      = the path to where the TK library is
#  TK_LIBRARY       = the name of the tk library found (tk tk80 etc)
#  TK_INCLUDE_PATH  = the path to where tk.h can be found
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

# try to find the Tk libraries in a few places and names
IF (NOT TK_LIB_PATH)
  FIND_LIBRARY(TK_LIB_PATH tk "C:/Program Files/Tcl/lib" /usr/lib /usr/local/lib)
  IF (TK_LIB_PATH)
    SET (TK_LIBRARY tk CACHE)
  ENDIF (TK_LIB_PATH)
ENDIF (NOT TK_LIB_PATH)

IF (NOT TK_LIB_PATH)
  FIND_LIBRARY(TK_LIB_PATH tk82 "C:/Program Files/Tcl/lib" /usr/lib /usr/local/lib)
  IF (TK_LIB_PATH)
    SET (TK_LIBRARY tk82 CACHE)
  ENDIF (TK_LIB_PATH)
ENDIF (NOT TK_LIB_PATH)

IF (NOT TK_LIB_PATH)
  FIND_LIBRARY(TK_LIB_PATH tk80 "C:/Program Files/Tcl/lib" /usr/lib /usr/local/lib)
  IF (TK_LIB_PATH)
    SET (TK_LIBRARY tk80 CACHE)
  ENDIF (TK_LIB_PATH)
ENDIF (NOT TK_LIB_PATH)

# add in the include path    
FIND_PATH(TCL_INCLUDE_PATH tcl.h "C:/Program Files/Tcl/include" /usr/include /usr/local/include)
FIND_PATH(TK_INCLUDE_PATH tk.h "C:/Program Files/Tcl/include" /usr/include /usr/local/include)
