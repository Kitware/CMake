# This is run by test test1 in repeat-after-timeout-cmake.cmake with cmake -P.
# It reads the file TEST_OUTPUT_FILE and increments the number
# found in the file by 1.  Unless the number is 2, then the
# code sends out a cmake error causing the test to not timeout only on
# the second time it is run.
message("TEST_OUTPUT_FILE = ${TEST_OUTPUT_FILE}")
file(READ "${TEST_OUTPUT_FILE}" COUNT)
message("COUNT= ${COUNT}")
math(EXPR COUNT "${COUNT} + 1")
file(WRITE "${TEST_OUTPUT_FILE}" "${COUNT}")
if(NOT COUNT EQUAL 2)
  message("this test times out except on the 2nd run")
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 10)
endif()
