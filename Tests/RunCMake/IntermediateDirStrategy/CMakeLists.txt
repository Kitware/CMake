cmake_minimum_required(VERSION 3.10)
project(${RunCMake_TEST} NONE)

if (NOT RunCMake_TEST MATCHES "IntDirStrategy")
  include(${RunCMake_TEST}.cmake)
endif ()
