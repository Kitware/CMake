#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TCL_TCLSH        = the full path to the tclsh binary (tcl tcl80 etc)
#

# In cygwin, look for the cygwin version first.  Don't look for it later to
# avoid finding the cygwin version on a Win32 build.
IF(WIN32)
  IF(UNIX)
    FIND_PROGRAM(TCL_TCLSH cygtclsh80)
  ENDIF(UNIX)
ENDIF(WIN32)

FIND_PROGRAM(TCL_TCLSH
  NAMES tclsh
  tclsh84 tclsh8.4
  tclsh83 tclsh8.3
  tclsh82 tclsh8.2
  tclsh80 tclsh8.0
)

IF (WIN32)
  MARK_AS_ADVANCED(
    TCL_TCLSH
    )
ENDIF(WIN32)
