# a simple C only test case
cmake_minimum_required(VERSION 2.8.12)
project (COnly C)

set(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")
add_library(testc1 STATIC libc1.c)
add_library(testc2 SHARED libc2.c)
add_executable (COnly conly.c foo.c foo.h)
target_link_libraries(COnly testc1 testc2)
if(MSVC_VERSION AND NOT CMAKE_C_COMPILER_ID STREQUAL Clang OR "x${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
  set_target_properties(COnly PROPERTIES
    LINK_FLAGS " /NODEFAULTLIB:\"libcdg.lib\" /NODEFAULTLIB:\"libcmtg.lib\" /NODEFAULTLIB:\"foomsvcrt.lib\" /NODEFAULTLIB:\"libbar.lib\" /NODEFAULTLIB:\"libfooba.lib\"")
endif()
string(ASCII 35 32 67 77 97 107 101 ASCII_STRING)
message(STATUS "String: ${ASCII_STRING}")

add_library(testCModule MODULE testCModule.c)
