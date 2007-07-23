# - Find wget
# This module looks for wget. This module defines the 
# following values:
#  WGET_EXECUTABLE: the full path to the wget tool.
#  WGET_FOUND: True if wget has been found.

INCLUDE(FindCygwin)

FIND_PROGRAM(WGET_EXECUTABLE
  wget
  ${CYGWIN_INSTALL_PATH}/bin
)

# handle the QUIETLY and REQUIRED arguments and set WGET_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Wget DEFAULT_MSG WGET_EXECUTABLE)

MARK_AS_ADVANCED( WGET_EXECUTABLE )

# WGET option is deprecated.
# use WGET_EXECUTABLE instead.
SET (WGET ${WGET_EXECUTABLE} )
