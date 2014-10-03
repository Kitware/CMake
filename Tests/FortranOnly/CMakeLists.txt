cmake_minimum_required (VERSION 2.8)
project(FortranOnly Fortran)
message("CTEST_FULL_OUTPUT ")

# create a library with hello and world functions
add_library(FortranOnlylib hello.f world.f)
set_property(TARGET FortranOnlylib PROPERTY Fortran_FORMAT FIXED)
set_property(SOURCE world.f PROPERTY Fortran_FORMAT FREE)

# create an executable that calls hello and world
add_executable(FortranOnly1 testf.f)
set_property(TARGET FortranOnly1 PROPERTY OUTPUT_NAME FortranOnly)
target_link_libraries(FortranOnly1 FortranOnlylib)

# create a custom command that runs FortranOnly1 and puts
# the output into the file testfhello.txt
add_custom_command(OUTPUT ${FortranOnly_BINARY_DIR}/testfhello.txt
  COMMAND FortranOnly1
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

# create a custom target that runs FortranOnly1 exectuable and creates
# a file out.txt that should have hello world in it.
add_custom_target(sayhello ALL
  COMMAND FortranOnly1 > out.txt
)
# make sure stuff is built in the right order
add_dependencies(checktestf2 FortranOnly2)
add_dependencies(sayhello FortranOnly1)
add_dependencies(FortranOnly2 FortranOnly1)

# add a custom target that checkes that out.txt has the correct
# content
add_custom_target(checksayhello ALL
  COMMAND ${CMAKE_COMMAND} -P ${FortranOnly_SOURCE_DIR}/checksayhello.cmake
  )
add_dependencies(checksayhello sayhello)

# Exclude this test on IBM XL for now because the check strangely
# fails with 'ld: 0706-029 Use a number with the -H flag'.
if(NOT CMAKE_Fortran_COMPILER_ID STREQUAL XL)
  set(err_log ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log)
  file(REMOVE "${err_log}")
  include(CheckFortranSourceCompiles)
  unset(HAVE_PRINT CACHE)
  CHECK_Fortran_SOURCE_COMPILES([[
      PROGRAM TEST_HAVE_PRINT
        PRINT *, 'Hello'
      END
]] HAVE_PRINT)
  if(NOT HAVE_PRINT)
    if(EXISTS "${err_log}")
      file(READ "${err_log}" err)
    endif()
    string(REPLACE "\n" "\n  " err "  ${err}")
    message(SEND_ERROR "CHECK_Fortran_SOURCE_COMPILES for HAVE_PRINT failed:\n"
      "${err}")
  endif()
endif()
