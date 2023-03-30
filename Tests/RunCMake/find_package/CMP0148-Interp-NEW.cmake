cmake_policy(SET CMP0148 NEW)
set(_FindPythonInterp_testing TRUE)
find_package(PythonInterp MODULE)

if(_FindPythonInterp_included)
  message(FATAL_ERROR "FindPythonInterp.cmake erroneously included")
endif()
