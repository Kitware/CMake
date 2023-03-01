cmake_minimum_required (VERSION 3.5)
project(CTestTestCrash)
include(CTest)

add_executable (Crash crash.cxx)

add_test (TestCrash Crash)
