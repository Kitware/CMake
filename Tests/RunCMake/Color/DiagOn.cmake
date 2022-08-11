set(EXPECT_COLOR 1)
include(DiagCommon.cmake)

if(DEFINED CMAKE_COLOR_MAKEFILE)
  message(FATAL_ERROR "CMAKE_COLOR_MAKEFILE incorrectly defined.")
endif()
