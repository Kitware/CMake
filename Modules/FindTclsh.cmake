#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TCL_TCLSH        = the full path to the tclsh binary (tcl tcl80 etc)
#  TK_WISH          = the full path to the wish binary (wish wish80 etc)
#

# if unix look for the cyg version first to avoid finding it 
# on a windows box running only win32 builds
IF(UNIX)
  FIND_PROGRAM(TCL_TCLSH cygtclsh80)
ENDIF(UNIX)

FIND_PROGRAM(TCL_TCLSH
  NAMES tclsh tclsh84 tclsh8.4 tclsh83 tclsh8.3 tclsh82 tclsh80 cygtclsh80
)

