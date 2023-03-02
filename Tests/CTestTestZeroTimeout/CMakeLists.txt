cmake_minimum_required (VERSION 3.5)
project (CTestTestZeroTimeout)
include (CTest)

add_executable (Sleep sleep.c)

add_test (TestExplicitZeroTimeout Sleep)
set_tests_properties(TestExplicitZeroTimeout PROPERTIES TIMEOUT 0)
