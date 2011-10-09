cmake_minimum_required(VERSION 2.8)

project(shared_lib)

set(CMAKE_VERBOSE_MAKEFILE ON)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/input.cpp"
  "int doNothing() { return 0; }\n"
)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
add_library(lib SHARED "${CMAKE_CURRENT_BINARY_DIR}/input.cpp")
