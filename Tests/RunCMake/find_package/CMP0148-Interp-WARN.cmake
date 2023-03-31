set(_FindPythonInterp_testing TRUE)
find_package(PythonInterp MODULE)

if(NOT _FindPythonInterp_included)
  message(FATAL_ERROR "FindPythonInterp.cmake not included")
endif()
