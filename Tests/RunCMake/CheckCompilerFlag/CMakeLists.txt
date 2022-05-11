cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()

project(${RunCMake_TEST} LANGUAGES NONE)

include(${RunCMake_TEST}.cmake)
