message("This is install script 4.")
if(INSTALL_SCRIPT_3_DID_RUN)
  message("Install script ordering works.")
else(INSTALL_SCRIPT_3_DID_RUN)
  message(FATAL_ERROR "Install script 3 did not run before install script 4.")
endif(INSTALL_SCRIPT_3_DID_RUN)
if(INSTALL_CODE_WITH_COMPONENT_DID_RUN)
  message("Install code ordering works.")
else(INSTALL_CODE_WITH_COMPONENT_DID_RUN)
  message(FATAL_ERROR "Install script 4 did not run after install with component code.")
endif(INSTALL_CODE_WITH_COMPONENT_DID_RUN)

if(CMAKE_INSTALL_COMPONENT)
if(NOT "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Development")
  message("CMAKE_INSTALL_COMPONENT=\"${CMAKE_INSTALL_COMPONENT}\"")
  message(FATAL_ERROR "Install script 4 should only run for \"Development\" INSTALL COMPONENT.")
endif(NOT "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Development")
endif(CMAKE_INSTALL_COMPONENT)

file(WRITE "${CMAKE_INSTALL_PREFIX}/MyTest/InstallScript4Out.cmake"
  "set(CMAKE_INSTALL_SCRIPT_4_DID_RUN 1)\n"
  )
