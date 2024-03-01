cmake_minimum_required (VERSION 3.28)
project(CheckLanguageHIPPlatform NONE)
include(CheckLanguage)

check_language(HIP)

if(NOT DEFINED CMAKE_HIP_COMPILER)
  message(FATAL_ERROR "check_language did not set result")
endif()

if (NOT CMAKE_HIP_COMPILER)
  message(FATAL_ERROR "check_language should not fail!")
endif()

if (NOT DEFINED CMAKE_HIP_PLATFORM)
  message(FATAL_ERROR "check_language did not set CMAKE_HIP_PLATFORM!")
endif()
