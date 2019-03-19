include("${CMAKE_CURRENT_LIST_DIR}/dir/dirtest.cmake")
include(CMakeParseArguments)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated.cmake" "")
include("${CMAKE_CURRENT_BINARY_DIR}/generated.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../FileAPIDummyFile.cmake")

add_subdirectory(dir)
