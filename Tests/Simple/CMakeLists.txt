# a simple test case
PROJECT (simple)
ADD_EXECUTABLE (simple simple)
ADD_LIBRARY(simpleLib STATIC simpleLib.cxx)
TARGET_LINK_LIBRARIES(simple simpleLib)
