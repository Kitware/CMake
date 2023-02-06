cmake_minimum_required(VERSION 3.5)
if(NOT RunCMake_TEST)
  set(RunCMake_TEST "$ENV{RunCMake_TEST}") # needed when cache is deleted
endif()
project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)
