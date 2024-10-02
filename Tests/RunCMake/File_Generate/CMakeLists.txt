cmake_minimum_required(VERSION 3.10)
if(RunCMake_TEST MATCHES "CMP0070-(WARN|OLD)")
  cmake_policy(VERSION 3.9) # old enough to not set CMP0070
endif()
project(${RunCMake_TEST} NONE)
if(NOT TEST_FILE)
  set(TEST_FILE ${RunCMake_TEST}.cmake)
endif()
include(${TEST_FILE})
