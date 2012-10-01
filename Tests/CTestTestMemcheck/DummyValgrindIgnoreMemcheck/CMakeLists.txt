cmake_minimum_required(VERSION 2.8.9)
project(CTestTestMemcheckDummyValgrindIgnoreMemcheck)
include(CTest)

add_test (RunCMake "${CMAKE_COMMAND}")
add_test (RunCMakeAgain "${CMAKE_COMMAND}")
