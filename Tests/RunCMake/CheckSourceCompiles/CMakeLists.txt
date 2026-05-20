cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0219)
  cmake_policy(SET CMP0219 OLD)
endif()

project(${RunCMake_TEST} LANGUAGES NONE)

include(${RunCMake_TEST}.cmake)
