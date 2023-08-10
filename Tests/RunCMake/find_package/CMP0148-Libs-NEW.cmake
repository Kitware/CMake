cmake_policy(SET CMP0148 NEW)
set(_FindPythonLibs_testing TRUE)
find_package(PythonLibs MODULE)

if(_FindPythonLibs_included)
  message(FATAL_ERROR "FindPythonLibs.cmake erroneously included")
endif()
