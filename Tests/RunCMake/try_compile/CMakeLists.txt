cmake_minimum_required(VERSION 3.10)
if(RunCMake_TEST STREQUAL "CMP0066")
  cmake_policy(VERSION 3.6) # old enough to not set CMP0066
elseif(RunCMake_TEST STREQUAL "CMP0067")
  cmake_policy(VERSION 3.7) # old enough to not set CMP0067
endif()
project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)
