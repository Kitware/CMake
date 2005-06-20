# a simple C only test case
project (conly C)

set(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")
add_library(testc1 STATIC libc1.c)
add_library(testc2 SHARED libc2.c)
add_executable (conly conly.c foo.c foo.h)
target_link_libraries(conly testc1 testc2)

string(ASCII 35 32 67 77 97 107 101 ASCII_STRING)
message(STATUS "String: ${ASCII_STRING}")
