cmake_minimum_required (VERSION 2.6)
project(CTestTestMemcheckDummyValgrindPrePost)
include(CTest)

add_test (RunCMake "${CMAKE_COMMAND}")
