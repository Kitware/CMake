# 
# this module looks for Cygwin and some usual commands
#

FIND_PATH(CYGWIN_INSTALL_PATH
  cygwin.bat
  "C:/Cygwin" 
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/;native]"
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

