cmake_minimum_required (VERSION 2.8.12)
project(CTestTestCrash)
include(CTest)

add_executable (Crash crash.cxx)

add_test (TestCrash Crash)
