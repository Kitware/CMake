cmake_minimum_required(VERSION 3.10)
if(RunCMake_TEST MATCHES "CMP0063-(OLD|WARN)")
  cmake_policy(VERSION 3.2) # old enough to not set CMP0063
endif()
project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)
