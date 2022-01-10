cmake_minimum_required(VERSION 3.16)
project(MocCMP0100)
include("../AutogenCoreTest.cmake")

set(CMAKE_AUTOMOC ON)
set(CSD ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(OLD)
add_subdirectory(NEW)
