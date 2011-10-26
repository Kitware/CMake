message("This is install script 2.")
if(INSTALL_SCRIPT_1_DID_RUN)
  message("Install script ordering works.")
else(INSTALL_SCRIPT_1_DID_RUN)
  message(FATAL_ERROR "Install script 1 did not run before install script 2.")
endif(INSTALL_SCRIPT_1_DID_RUN)
if(INSTALL_CODE_DID_RUN)
  message("Install code ordering works.")
else(INSTALL_CODE_DID_RUN)
  message(FATAL_ERROR "Install script 2 did not run after install code.")
endif(INSTALL_CODE_DID_RUN)
file(WRITE "${CMAKE_INSTALL_PREFIX}/MyTest/InstallScriptOut.cmake"
  "set(CMAKE_INSTALL_SCRIPT_DID_RUN 1)\n"
  )
