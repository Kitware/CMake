cmake_minimum_required (VERSION 2.6)
PROJECT(CTestTestCrash)
INCLUDE(CTest)

ADD_EXECUTABLE (Crash crash.cxx)

ADD_TEST (TestCrash Crash)
