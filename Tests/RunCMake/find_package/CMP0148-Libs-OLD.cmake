cmake_policy(SET CMP0148 OLD)
set(_FindPythonLibs_testing TRUE)
find_package(PythonLibs MODULE)

if(NOT _FindPythonLibs_included)
  message(FATAL_ERROR "FindPythonLibs.cmake not included")
endif()
