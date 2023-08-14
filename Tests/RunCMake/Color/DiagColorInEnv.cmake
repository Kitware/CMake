if (CMAKE_GENERATOR MATCHES "Makefiles")
  set(CMAKE_COLOR_DIAGNOSTICS 1)
  set(EXPECT_COLOR 1)
endif ()
include(DiagCommon.cmake)

if(CMAKE_GENERATOR MATCHES "Makefiles" AND NOT DEFINED CMAKE_COLOR_MAKEFILE)
  message(FATAL_ERROR "CMAKE_COLOR_MAKEFILE incorrectly undefined.")
endif()
