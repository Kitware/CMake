cmake_minimum_required(VERSION 3.5)
project(${RunCMake_TEST} NONE)

message(STATUS "source: '${CMAKE_SOURCE_DIR}'")
message(STATUS "binary: '${CMAKE_BINARY_DIR}'")
get_filename_component(real_source "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(real_binary "${CMAKE_BINARY_DIR}" REALPATH)
message(STATUS "real source: '${real_source}'")
message(STATUS "real binary: '${real_binary}'")

if(RunCMake_TEST MATCHES "-exe")
  enable_language(C)
  file(WRITE "${CMAKE_SOURCE_DIR}/source.c" [[
    #include <stdio.h>
    #include "source.h"
    #include "binary.h"
    extern void print_binary_c(void);
    extern void print_binary_c(void);
    void print_source_c(void) {
      printf("source.c: '%s'\n", __FILE__);
    }
    int main(void) {
      print_source_c();
      print_source_h();
      print_binary_c();
      print_binary_h();
      return 0;
    }
  ]])
  file(WRITE "${CMAKE_BINARY_DIR}/binary.c" [[
    #include <stdio.h>
    void print_binary_c(void) {
      printf("binary.c: '%s'\n", __FILE__);
    }
  ]])
  file(WRITE "${CMAKE_SOURCE_DIR}/include/source.h" [[
    void print_source_h(void) {
      printf("source.h: '%s'\n", __FILE__);
    }
  ]])
  file(WRITE "${CMAKE_BINARY_DIR}/include/binary.h" [[
    void print_binary_h(void) {
      printf("binary.h: '%s'\n", __FILE__);
    }
  ]])
  add_executable(exe source.c ${CMAKE_BINARY_DIR}/binary.c)
  target_include_directories(exe PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    )
  add_custom_target(print ALL COMMAND exe)
endif()
