cmake_minimum_required (VERSION 2.6)
project(CTestTestMemcheckDummyValgrindFailPre)
include(CTest)

add_test (RunCMake "${CMAKE_COMMAND}")
