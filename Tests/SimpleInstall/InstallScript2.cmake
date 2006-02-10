MESSAGE("This is install script 2.")
IF(INSTALL_SCRIPT_1_DID_RUN)
  MESSAGE("Install script ordering works.")
ELSE(INSTALL_SCRIPT_1_DID_RUN)
  MESSAGE(FATAL_ERROR "Install script 1 did not run before install script 2.")
ENDIF(INSTALL_SCRIPT_1_DID_RUN)
FILE(WRITE "${CMAKE_INSTALL_PREFIX}/InstallScriptOut.cmake"
  "SET(CMAKE_INSTALL_SCRIPT_DID_RUN 1)\n"
  )
