# a simple C only test case
PROJECT (conly C)
ADD_LIBRARY(testc1 STATIC libc1.c)
ADD_LIBRARY(testc2 SHARED libc2.c)
ADD_EXECUTABLE (conly conly.c foo.c foo.h)
TARGET_LINK_LIBRARIES(conly testc1 testc2)

STRING(ASCII 35 32 67 77 97 107 101 ASCII_STRING)
MESSAGE(STATUS "String: ${ASCII_STRING}")
