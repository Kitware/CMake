# a simple test case
cmake_minimum_required (VERSION 3.3)
project (PreOrder)
set(CMAKE_IGNORE_DEPENDENCIES_ORDERING 1)
add_subdirectory(Library)
add_executable (PreOrder simple.cxx)
target_link_libraries(PreOrder simpleLib)
