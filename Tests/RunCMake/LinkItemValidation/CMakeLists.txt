cmake_minimum_required(VERSION 2.8.12)
if(NOT RunCMake_TEST MATCHES "^CMP0028")
  cmake_minimum_required(VERSION 3.22)
endif()
project(${RunCMake_TEST} CXX)
include(${RunCMake_TEST}.cmake NO_POLICY_SCOPE) # policy used at end of dir
