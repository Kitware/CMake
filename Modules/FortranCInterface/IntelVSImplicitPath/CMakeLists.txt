cmake_minimum_required (VERSION 2.8)
project(IntelFortranImplicit Fortran)
add_custom_command(OUTPUT ${IntelFortranImplicit_BINARY_DIR}/env.txt
  COMMAND set > ${IntelFortranImplicit_BINARY_DIR}/env.txt)
add_library(FortranLib hello.f
  ${IntelFortranImplicit_BINARY_DIR}/env.txt)
add_custom_target(ExtractLibPath ALL
  COMMAND ${CMAKE_COMMAND} -P ${IntelFortranImplicit_SOURCE_DIR}/extract.cmake
  WORKING_DIRECTORY ${IntelFortranImplicit_BINARY_DIR}
)
add_dependencies(ExtractLibPath FortranLib)
