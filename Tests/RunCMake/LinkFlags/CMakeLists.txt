cmake_minimum_required(VERSION 3.10)
project(${RunCMake_TEST} C CXX)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
set(obj_cxx "${CMAKE_CXX_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

include(${RunCMake_TEST}.cmake)
