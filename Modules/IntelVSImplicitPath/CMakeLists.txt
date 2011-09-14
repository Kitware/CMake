cmake_minimum_required (VERSION 2.8)
project(IntelFortranImplicit Fortran)
add_custom_command(
  OUTPUT output.cmake
  COMMAND ${CMAKE_COMMAND} -P ${IntelFortranImplicit_SOURCE_DIR}/detect.cmake
  )
add_library(FortranLib hello.f output.cmake)
