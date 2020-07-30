cmake_minimum_required(VERSION 3.16)
project(UnityMocSource)
include("../AutogenGuiTest.cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_UNITY_BUILD ON)

add_library(library library.cpp)
target_link_libraries(library ${QT_LIBRARIES})

add_executable(unityMocSource main.cpp)
target_link_libraries(unityMocSource PRIVATE library)
