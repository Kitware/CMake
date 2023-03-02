# a simple C only test case
cmake_minimum_required(VERSION 3.5)
project (COnly C)

set(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")
add_library(testc1 STATIC libc1.c)
add_library(testc2 SHARED libc2.c)
add_executable (COnly conly.c foo.c foo.h)
target_link_libraries(COnly testc1 testc2)

add_library(testCModule MODULE testCModule.c)
