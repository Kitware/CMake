cmake_minimum_required(VERSION 3.15)
project(PchReuseFrom C)

add_library(empty empty.c)
target_precompile_headers(empty PUBLIC
  <stdio.h>
  <string.h>
)
target_include_directories(empty PUBLIC include)

add_library(foo foo.c)
target_include_directories(foo PUBLIC include)
target_precompile_headers(foo REUSE_FROM empty)

add_executable(foobar foobar.c)
target_link_libraries(foobar foo )
set_target_properties(foobar PROPERTIES PRECOMPILE_HEADERS_REUSE_FROM foo)

enable_testing()
add_test(NAME foobar COMMAND foobar)
