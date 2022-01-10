cmake_minimum_required(VERSION 3.15)
project(PchIncludedAllLanguages C CXX)

if(CMAKE_CXX_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

add_executable(main
  main.cpp
  pch-included.c
  pch-included.cpp
)
target_precompile_headers(main PRIVATE pch.h)

enable_testing()
add_test(NAME main COMMAND main)
