cmake_minimum_required(VERSION 3.10)
project(TestPolicyCMP0065 C)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS BADFLAGS)

add_executable(FooNEWGood main.c)

add_executable(FooNEWBad main.c)
set_target_properties(FooNEWBad PROPERTIES ENABLE_EXPORTS ON)
