# a simple test case
PROJECT (simple)
ADD_EXECUTABLE (simple simple.cxx)
ADD_LIBRARY(simpleLib STATIC simpleLib.cxx simpleCLib.c)
TARGET_LINK_LIBRARIES(simple simpleLib)
