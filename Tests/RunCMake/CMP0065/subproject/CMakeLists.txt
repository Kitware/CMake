cmake_minimum_required(VERSION 3.2)

project(TestPolicyCMP0065 CXX)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS BADFLAGS)

#----------------------------------------------------------------------
cmake_policy(SET CMP0065 OLD)
add_executable(FooOLDBad1 main.cxx)

#----------------------------------------------------------------------
cmake_policy(SET CMP0065 NEW)
add_executable(FooOLDBad2 main.cxx)
set_target_properties(FooOLDBad2 PROPERTIES ENABLE_EXPORTS ON)

#----------------------------------------------------------------------
cmake_policy(SET CMP0065 NEW)
add_executable(FooNEWGood main.cxx)

#----------------------------------------------------------------------
cmake_policy(SET CMP0065 NEW)
add_executable(FooNEWBad main.cxx)
set_target_properties(FooNEWBad PROPERTIES ENABLE_EXPORTS ON)
