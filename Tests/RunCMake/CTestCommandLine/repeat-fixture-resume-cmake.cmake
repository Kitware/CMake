# `ctest -F` resumes from the checkpoint that records the tests already run.
cmake_policy(SET CMP0224 NEW)
include(${CMAKE_CURRENT_LIST_DIR}/repeat-fixture-common.cmake)

add_test(NAME not_yet_run COMMAND ${CMAKE_COMMAND} -E true)
