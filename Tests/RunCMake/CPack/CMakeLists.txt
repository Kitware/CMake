cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${RunCMake_TEST} CXX)

# set default test name (can be overwritten in test.cmake or specifics.cmake)
string(TOLOWER "${RunCMake_TEST_FILE_PREFIX}" CPACK_PACKAGE_NAME)

include(tests/${RunCMake_TEST_FILE_PREFIX}/test.cmake)

set(CPACK_GENERATOR "${GENERATOR_TYPE}")
include(CPack)
