cmake_minimum_required(VERSION 3.26)
project(AutoMocGeneratedFile)

include("../AutogenCoreTest.cmake")

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/myConfig.h
    CONTENT
    "
#ifndef MYCONFIG_H
#define MYCONFIG_H

inline void foo() {}

#endif

"
)

add_executable(testTarget
  main.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/myConfig.h)
target_include_directories(testTarget PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET testTarget PROPERTY AUTOMOC ON)
