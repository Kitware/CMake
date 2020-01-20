cmake_minimum_required(VERSION 3.15)
project(PchDebugGenex C)

add_library(foo foo.c)
target_include_directories(foo PUBLIC include)
target_precompile_headers(foo PUBLIC
  "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/include/foo.h>"
  <stdio.h>
)
