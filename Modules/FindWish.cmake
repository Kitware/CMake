#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TK_WISH          = the full path to the wish binary (wish wish80 etc)
#

# if UNIX is defined, then look for the cygwin version first
IF(UNIX)
  FIND_PROGRAM(TK_WISH cygwish80 )
ENDIF(UNIX)

FIND_PROGRAM(TK_WISH
  NAMES wish wish84 wish8.4 wish83 wish8.3 wish82 wish80
)

