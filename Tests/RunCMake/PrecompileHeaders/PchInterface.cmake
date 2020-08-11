cmake_minimum_required(VERSION 3.15)
project(PchInterface C)

add_library(foo foo.c)
target_include_directories(foo PUBLIC include)
target_precompile_headers(foo PUBLIC
  include/foo.h
  \"foo2.h\"
  <stdio.h>
  \"string.h\"
)
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set_property(SOURCE foo.c APPEND PROPERTY COMPILE_OPTIONS "-WX-")
endif()

add_library(bar INTERFACE)
target_include_directories(bar INTERFACE include)
target_precompile_headers(bar INTERFACE include/bar.h)

add_executable(foobar foobar.c)
target_link_libraries(foobar foo bar)

enable_testing()
add_test(NAME foobar COMMAND foobar)
