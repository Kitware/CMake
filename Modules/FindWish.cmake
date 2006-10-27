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

FIND_PROGRAM(TK_WISH
  NAMES wish wish84 wish8.4 wish83 wish8.3 wish82 wish8.2 wish80
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/bin
)

MARK_AS_ADVANCED(TK_WISH  )
