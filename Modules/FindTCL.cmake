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

FIND_LIBRARY(TCL_LIBRARY
  NAMES tcl tcl84 tcl83 tcl82 tcl80 
  PATHS  
  /usr/lib 
  /usr/local/lib
  "C:/Program Files/Tcl/lib" 
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root]/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.3;Root]/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.2;Root]/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.0;Root]/lib
)

FIND_LIBRARY(TK_LIBRARY 
  NAMES tk tk84 tk83 tk82 tk80
  PATHS 
  /usr/lib 
  /usr/local/lib
  "C:/Program Files/Tcl/lib" 
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root]/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.3;Root]/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.2;Root]/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.0;Root]/lib
)

FIND_PATH(TCL_INCLUDE_PATH tcl.h 
  /usr/include 
  /usr/local/include
  "C:/Program Files/Tcl/include" 
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.3;Root]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.2;Root]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.0;Root]/include
)

FIND_PATH(TK_INCLUDE_PATH tk.h 
  /usr/include 
  /usr/local/include
  "C:/Program Files/Tcl/include" 
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.3;Root]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.2;Root]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.0;Root]/include
)
