# 
# this module looks for wget
#

INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)

IF (CYGWIN_INSTALL_PATH)

  FIND_PROGRAM(WGET
    wget
    ${CYGWIN_INSTALL_PATH}/bin
  )

ELSE (CYGWIN_INSTALL_PATH)

  FIND_PROGRAM(WGET
    wget
  )

ENDIF (CYGWIN_INSTALL_PATH)
