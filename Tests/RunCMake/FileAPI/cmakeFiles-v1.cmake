include("${CMAKE_CURRENT_LIST_DIR}/dir/dirtest.cmake")
include(CMakeParseArguments)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated.cmake" "")
include("${CMAKE_CURRENT_BINARY_DIR}/generated.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../FileAPIDummyFile.cmake")

file(GLOB var
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/dir/*")

file(GLOB_RECURSE var
  FOLLOW_SYMLINKS
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/dir/*.cmake")

add_subdirectory(dir)
