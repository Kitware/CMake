cmake_minimum_required(VERSION 2.8)

project(CompileOptions)

add_library(testlib other.cpp)

add_executable(CompileOptions main.cpp)
set_property(TARGET CompileOptions PROPERTY COMPILE_OPTIONS
  "-DTEST_DEFINE"
  "-DNEEDS_ESCAPE=\"E$CAPE\""
  "$<$<CXX_COMPILER_ID:GNU>:-DTEST_DEFINE_GNU>"
  )
target_link_libraries(CompileOptions testlib)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_definitions(CompileOptions
    PRIVATE
      "DO_GNU_TESTS"
  )
endif()
