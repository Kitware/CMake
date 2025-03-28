cmake_minimum_required(VERSION 3.10)
project(${RunCMake_TEST} NONE)

if(CMP0168 STREQUAL "NEW")
  cmake_policy(SET CMP0168 NEW)
else()
  cmake_policy(SET CMP0168 OLD)
endif()
include(${RunCMake_TEST}.cmake)
