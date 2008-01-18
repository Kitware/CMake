# - Find wish installation
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TK_WISH = the path to the wish executable
#
# if UNIX is defined, then it will look for the cygwin version first
IF(UNIX)
  FIND_PROGRAM(TK_WISH cygwish80 )
ENDIF(UNIX)

GET_FILENAME_COMPONENT(TCL_INCLUDE_PATH_PARENT "${TCL_INCLUDE_PATH}" PATH)
GET_FILENAME_COMPONENT(TK_INCLUDE_PATH_PARENT "${TK_INCLUDE_PATH}" PATH)

GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)

GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH_PARENT "${TCL_LIBRARY_PATH}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH_PARENT "${TK_LIBRARY_PATH}" PATH)

GET_FILENAME_COMPONENT(
  ActiveTcl_CurrentVersion 
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl;CurrentVersion]" 
  NAME)

FIND_PROGRAM(TK_WISH
  NAMES wish 
  wish85 wish8.5 
  wish84 wish8.4 
  wish83 wish8.3 
  wish82 wish8.2 
  wish80 wish8.0
  PATHS
  "${TCL_INCLUDE_PATH_PARENT}/bin"
  "${TK_INCLUDE_PATH_PARENT}/bin"
  "${TCL_LIBRARY_PATH_PARENT}/bin"
  "${TK_LIBRARY_PATH_PARENT}/bin"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/bin
)

MARK_AS_ADVANCED(TK_WISH  )
