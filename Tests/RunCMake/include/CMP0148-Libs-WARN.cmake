# Do not set CMP0148.
set(_FindPythonLibs_testing 1)
include(FindPythonLibs)

if(NOT _FindPythonLibs_included)
  message(FATAL_ERROR "FindPythonLibs.cmake not included")
endif()
