#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TCL_LIBRARY      = the full path to the tcl library found (tcl tcl80 etc)
#  TCL_INCLUDE_PATH = the path to where tcl.h can be found
#  TK_LIBRARY       = the full path to the tk library found (tk tk80 etc)
#  TK_INCLUDE_PATH  = the path to where tk.h can be found
#

# try to find the Tcl libraries in a few places and names
FIND_LIBRARY(TCL_LIBRARY
             NAMES tcl tcl82 tcl80 
             PATHS  /usr/lib "C:/Program Files/Tcl/lib" /usr/local/lib)

FIND_LIBRARY(TK_LIBRARY 
             NAMES tk tk82 tk80
             PATHS /usr/lib "C:/Program Files/Tcl/lib" /usr/local/lib)

# add in the include path    
FIND_PATH(TCL_INCLUDE_PATH tcl.h "C:/Program Files/Tcl/include" /usr/include /usr/local/include)
FIND_PATH(TK_INCLUDE_PATH tk.h "C:/Program Files/Tcl/include" /usr/include /usr/local/include)
