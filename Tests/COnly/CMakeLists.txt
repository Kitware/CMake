# a simple C only test case
PROJECT (conly C)
ADD_LIBRARY(c1 STATIC libc1.c)
ADD_LIBRARY(c2 SHARED libc2.c)
ADD_EXECUTABLE (conly conly.c foo.c foo.h)
TARGET_LINK_LIBRARIES(conly c1 c2)
