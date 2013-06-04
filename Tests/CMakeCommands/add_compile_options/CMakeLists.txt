cmake_minimum_required(VERSION 2.8)

project(add_compile_options)

add_compile_options(-DTEST_OPTION)

add_executable(add_compile_options main.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_definitions(add_compile_options
    PRIVATE
      "DO_GNU_TESTS"
  )
endif()
