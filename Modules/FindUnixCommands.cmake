# 
# this module looks for some usual Unix commands
#

INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)

IF (CYGWIN_INSTALL_PATH)

  FIND_PROGRAM(BASH
    bash
    ${CYGWIN_INSTALL_PATH}/bin
  )

  FIND_PROGRAM(CP
    cp
    ${CYGWIN_INSTALL_PATH}/bin
  )

  FIND_PROGRAM(GZIP
    gzip
    ${CYGWIN_INSTALL_PATH}/bin
  )

  FIND_PROGRAM(MV
    mv
    ${CYGWIN_INSTALL_PATH}/bin
  )

  FIND_PROGRAM(RM
    rm
    ${CYGWIN_INSTALL_PATH}/bin
  )

  FIND_PROGRAM(TAR
    NAMES 
    tar 
    gtar
    PATH
    ${CYGWIN_INSTALL_PATH}/bin
  )

ELSE (CYGWIN_INSTALL_PATH)

  FIND_PROGRAM(BASH
    bash
    /bin
    /usr/bin 
    /usr/local/bin
    /sbin
  )

  FIND_PROGRAM(CP
    cp
    /bin
    /usr/bin 
    /usr/local/bin
    /sbin
  )

  FIND_PROGRAM(GZIP
    gzip
    /bin
    /usr/bin 
    /usr/local/bin
    /sbin
  )

  FIND_PROGRAM(MV
    mv
    /bin
    /usr/bin 
    /usr/local/bin
    /sbin
  )

  FIND_PROGRAM(RM
    rm
    /bin
    /usr/bin 
    /usr/local/bin
    /sbin
  )

  FIND_PROGRAM(TAR
    NAMES 
    tar 
    gtar
    PATH
    /bin
    /usr/bin 
    /usr/local/bin
    /sbin
  )

ENDIF (CYGWIN_INSTALL_PATH)
