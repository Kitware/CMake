cmake_minimum_required(VERSION 3.12...3.13)

project(TestBasicFortran CXX Fortran)

include(CTest)

set(language "fortran")

include (../BasicConfiguration.cmake)
get_target_property(EXAMPLE_PREFIX example PREFIX)
if (NOT EXAMPLE_PREFIX STREQUAL "${CMAKE_SHARED_LIBRARY_PREFIX}"
    AND NOT EXAMPLE_PREFIX STREQUAL "EXAMPLE_PREFIX-NOTFOUND")
  message(FATAL_ERROR "Unexpected library prefix on target: got "
    "'${EXAMPLE_PREFIX}' but expected '${CMAKE_SHARED_LIBRARY_PREFIX}'")
endif()


add_executable(runme ${CMAKE_CURRENT_SOURCE_DIR}/../runme.f90)
target_link_libraries(runme example)
set_target_properties(runme PROPERTIES LINKER_LANGUAGE Fortran)

add_test (NAME BasicFortran
  COMMAND $<TARGET_FILE:runme>)
