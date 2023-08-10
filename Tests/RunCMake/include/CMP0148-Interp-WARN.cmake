# Do not set CMP0148.
set(_FindPythonInterp_testing 1)
include(FindPythonInterp)

if(NOT _FindPythonInterp_included)
  message(FATAL_ERROR "FindPythonInterp.cmake not included")
endif()
