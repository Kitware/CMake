# 
# this module looks for wget
#

INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)

FIND_PROGRAM(WGET
  wget
  ${CYGWIN_INSTALL_PATH}/bin
)

MARK_AS_ADVANCED(
  WGET
)
