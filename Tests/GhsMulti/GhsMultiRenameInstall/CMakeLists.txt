# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

set(targets_to_install "")

if(CMAKE_C_COMPILER_ID STREQUAL "GHS")
  add_link_options("-non_shared")
endif()

if(RUN_TEST STREQUAL "SINGLE_EXEC")
  add_executable(exe1 exe.c)
  set(targets_to_install exe1)
endif()

if(RUN_TEST STREQUAL "SINGLE_EXEC_RENAMED")
  set(name new_name)
  add_executable(exe1 exe.c)
  set_property(TARGET exe1 PROPERTY RUNTIME_OUTPUT_DIRECTORY ${name}_bin_$<CONFIG>)
  set_property(TARGET exe1 PROPERTY OUTPUT_NAME ${name}_$<CONFIG>)
  set_property(TARGET exe1 PROPERTY SUFFIX .bin)
  set(targets_to_install exe1)
endif()

if(RUN_TEST STREQUAL "EXEC_AND_LIB")
 add_library(lib1 lib1.c)
 set_property(TARGET lib1 PROPERTY ARCHIVE_OUTPUT_DIRECTORY forced-$<CONFIG>)
 set_property(TARGET lib1 PROPERTY SUFFIX .LL)
 set_property(TARGET lib1 PROPERTY OUTPUT_NAME lib1_$<CONFIG>)

 add_executable(exe1 exe1.c)
 target_link_libraries(exe1 lib1)
 set(targets_to_install exe1 lib1)
endif()

install(TARGETS ${targets_to_install}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
