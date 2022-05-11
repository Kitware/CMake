cmake_policy(SET CMP0076 NEW)
enable_language(CXX)

add_library(privatelib)

include("RelativePathInSubdirInclude/CMakeLists.txt")

get_property(privatelib_sources TARGET privatelib PROPERTY SOURCES)
message(STATUS "privatelib: ${privatelib_sources}")
