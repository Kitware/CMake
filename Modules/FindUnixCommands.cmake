# - Find unix commands from cygwin
# This module looks for some usual Unix commands.
#

INCLUDE(FindCygwin)

FIND_PROGRAM(BASH
  bash
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  BASH
)

FIND_PROGRAM(CP
  cp
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  CP
)

FIND_PROGRAM(GZIP
  gzip
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  GZIP
)

FIND_PROGRAM(MV
  mv
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  MV
)

FIND_PROGRAM(RM
  rm
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  RM
)

FIND_PROGRAM(TAR
  NAMES 
  tar 
  gtar
  PATH
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  TAR
)
