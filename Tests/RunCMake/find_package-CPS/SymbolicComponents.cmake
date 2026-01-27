cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

add_library(bar INTERFACE)

find_package(Symbolic REQUIRED COMPONENTS foo test)
target_link_libraries(bar INTERFACE Symbolic::foo Symbolic::test)
