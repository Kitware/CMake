cmake_minimum_required(VERSION 3.5)
project(test C)

add_library(libdemo
  test.c
  testCase.c
  subfolder_test.c
  subfolder_test_0.c
  "subfolder/test.c"
  subfolder/testcase.c
)

add_executable(demo main.c)
target_link_libraries(demo libdemo)
if(CMAKE_C_COMPILER_ID STREQUAL "GHS")
  target_link_options(demo PRIVATE "-non_shared")
endif()
