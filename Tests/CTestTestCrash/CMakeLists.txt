cmake_minimum_required(VERSION 3.10)
project(CTestTestCrash)
include(CTest)

add_executable (Crash crash.cxx)

add_test (TestCrash Crash)
