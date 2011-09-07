cmake_minimum_required (VERSION 2.8)
project(FortranOnly Fortran)
message("CTEST_FULL_OUTPUT ")

# create a library with hello and world functions
add_library(FortranOnlylib hello.f world.f)
set_property(TARGET FortranOnlylib PROPERTY Fortran_FORMAT FIXED)
set_property(SOURCE world.f PROPERTY Fortran_FORMAT FREE)

# create an executable that calls hello and world
add_executable(FortranOnly testf.f)
target_link_libraries(FortranOnly FortranOnlylib)

# create a custom command that runs FortranOnly and puts
# the output into the file testfhello.txt
add_custom_command(OUTPUT ${FortranOnly_BINARY_DIR}/testfhello.txt
  COMMAND ${FortranOnly_BINARY_DIR}/${CMAKE_CFG_INTDIR}/FortranOnly
  > testfhello.txt)
# create a second executable FortranOnly2 that has
# testfhello.txt has an source file so that it will
# run the above custom command.
add_executable(FortranOnly2 testfhello.txt testf.f)
target_link_libraries(FortranOnly2 FortranOnlylib)
# create a custom target to check the content of testfhello.txt
# by running the cmake script checktestf2.cmake
add_custom_target(checktestf2 ALL
  COMMAND ${CMAKE_COMMAND}
  -P ${FortranOnly_SOURCE_DIR}/checktestf2.cmake)

# create a custom target that runs FortranOnly exectuable and creates
# a file out.txt that should have hello world in it.
add_custom_target(sayhello ALL
  COMMAND ${FortranOnly_BINARY_DIR}/${CMAKE_CFG_INTDIR}/FortranOnly > out.txt
)
# make sure stuff is built in the right order
add_dependencies(checktestf2 FortranOnly2)
add_dependencies(sayhello FortranOnly)
add_dependencies(FortranOnly2 FortranOnly)

# add a custom target that checkes that out.txt has the correct
# content
add_custom_target(checksayhello ALL
  COMMAND ${CMAKE_COMMAND} -P ${FortranOnly_SOURCE_DIR}/checksayhello.cmake
  )
add_dependencies(checksayhello sayhello)
