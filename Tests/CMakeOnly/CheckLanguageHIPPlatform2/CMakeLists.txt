cmake_minimum_required (VERSION 3.28)
project(CheckLanguageHIPPlatform2 NONE)
include(CheckLanguage)

set(CMAKE_HIP_PLATFORM "not-a-hip-platform" CACHE STRING "")
check_language(HIP)

if(NOT DEFINED CMAKE_HIP_COMPILER)
  message(FATAL_ERROR "check_language did not set result")
endif()

if (CMAKE_HIP_COMPILER)
  message(FATAL_ERROR "check_language should have failed")
endif()
