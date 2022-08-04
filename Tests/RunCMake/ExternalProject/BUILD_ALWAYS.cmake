include(ExternalProject)

ExternalProject_Add(once
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/once-configure.cmake
  BUILD_COMMAND "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/once-build.cmake
  INSTALL_COMMAND "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/once-install.cmake
  )

ExternalProject_Add(always
  DEPENDS once
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/always-configure.cmake
  BUILD_COMMAND "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/always-build.cmake
  BUILD_ALWAYS 1
  INSTALL_COMMAND "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/always-install.cmake
  )
