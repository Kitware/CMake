MESSAGE("This is install script 2.")
IF(INSTALL_SCRIPT_1_DID_RUN)
  MESSAGE("Install script ordering works.")
ELSE(INSTALL_SCRIPT_1_DID_RUN)
  MESSAGE(FATAL_ERROR "Install script 1 did not run before install script 2.")
ENDIF(INSTALL_SCRIPT_1_DID_RUN)
IF(INSTALL_CODE_DID_RUN)
  MESSAGE("Install code ordering works.")
ELSE(INSTALL_CODE_DID_RUN)
  MESSAGE(FATAL_ERROR "Install script 2 did not run after install code.")
ENDIF(INSTALL_CODE_DID_RUN)
FILE(WRITE "${CMAKE_INSTALL_PREFIX}/MyTest/InstallScriptOut.cmake"
  "SET(CMAKE_INSTALL_SCRIPT_DID_RUN 1)\n"
  )
