cmake_policy(SET CMP0225 NEW)
include(ExternalProject)
ExternalProject_Add(Sub
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Sub"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Sub-build"
  INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install dir"
  CMAKE_ARGS "--install-prefix=${CMAKE_CURRENT_BINARY_DIR}/override"
  DOWNLOAD_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)
