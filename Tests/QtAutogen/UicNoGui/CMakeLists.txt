cmake_minimum_required(VERSION 3.13)
project(UicNoGui)
include("../AutogenCoreTest.cmake")

# This tests creates a target that has AUTOUIC enabled but does not
# link against QtXWidgets.

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(MocOnly)
add_subdirectory(NoQt)

add_executable(uicNoGui main.cpp)
target_link_libraries(uicNoGui mocOnly noQt)
