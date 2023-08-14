cmake_policy(SET CMP0148 OLD)
set(_FindPythonLibs_testing 1)
include(FindPythonLibs)

if(NOT _FindPythonLibs_included)
  message(FATAL_ERROR "FindPythonLibs.cmake not included")
endif()
