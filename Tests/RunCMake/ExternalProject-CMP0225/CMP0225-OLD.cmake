cmake_policy(SET CMP0225 OLD)
include(ExternalProject)
ExternalProject_Add(Sub
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/Sub"
  TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp"
  DOWNLOAD_COMMAND ""
)
