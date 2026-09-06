cmake_policy(SET CMP0225 NEW)
include(ExternalProject)
ExternalProject_Add(Sub
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Sub"
  TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp"
  CONFIGURE_COMMAND ""
  DOWNLOAD_COMMAND ""
)
