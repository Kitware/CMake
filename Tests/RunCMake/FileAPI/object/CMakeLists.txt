cmake_minimum_required(VERSION 3.13)
project(Object)
enable_language(CXX)

cmake_policy(SET CMP0203 NEW)
cmake_policy(SET CMP0204 NEW)

add_library(c_object_lib OBJECT ../empty.c)
add_executable(c_object_exe ../empty.c)
target_link_libraries(c_object_exe PRIVATE c_object_lib)

add_library(cxx_object_lib OBJECT ../empty.cxx)
add_executable(cxx_object_exe ../empty.cxx)
target_link_libraries(cxx_object_exe PRIVATE cxx_object_lib)

install(TARGETS c_object_exe cxx_object_exe DESTINATION bin)
