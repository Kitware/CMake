include(DiagCommon.cmake)

if(DEFINED CMAKE_COLOR_DIAGNOSTICS)
  message(FATAL_ERROR "CMAKE_COLOR_DIAGNOSTICS incorrectly defined.")
endif()
if(CMAKE_GENERATOR MATCHES "Make" AND NOT DEFINED CMAKE_COLOR_MAKEFILE)
  message(FATAL_ERROR "CMAKE_COLOR_MAKEFILE incorrectly not defined.")
endif()
