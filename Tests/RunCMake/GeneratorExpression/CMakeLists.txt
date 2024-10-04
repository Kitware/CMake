cmake_minimum_required(VERSION 3.10)
if(RunCMake_TEST STREQUAL "CMP0044-WARN")
  cmake_policy(VERSION 2.8.11) # old enough to not set CMP0044
endif()
project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)
