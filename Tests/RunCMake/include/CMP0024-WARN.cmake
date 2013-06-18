
enable_language(CXX)

add_library(foo SHARED empty.cpp)

export(TARGETS foo FILE "${CMAKE_CURRENT_BINARY_DIR}/theTargets.cmake")
include("${CMAKE_CURRENT_BINARY_DIR}/theTargets.cmake")
