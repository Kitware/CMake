
enable_language(CXX)

cmake_policy(SET CMP0024 NEW)

add_library(foo SHARED empty.cpp)

export(TARGETS foo FILE "${CMAKE_CURRENT_BINARY_DIR}/theTargets.cmake")
include("${CMAKE_CURRENT_BINARY_DIR}/theTargets.cmake")
