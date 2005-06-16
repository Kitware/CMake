# a target with the same name as a target in a different dir
ADD_EXECUTABLE(mytest_exe conly.c)
SET_TARGET_PROPERTIES(mytest_exe PROPERTIES OUTPUT_NAME mytest)
TARGET_LINK_LIBRARIES(mytest_exe mytest)

# and two targets in the same dir with the same name
ADD_LIBRARY(mytest2 ../Lib1/libc1.c)

ADD_EXECUTABLE(mytest2_exe conly.c)
SET_TARGET_PROPERTIES(mytest2_exe PROPERTIES OUTPUT_NAME mytest2)
TARGET_LINK_LIBRARIES(mytest2_exe mytest2)
