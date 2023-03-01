cmake_minimum_required(VERSION 3.5)
project(VSMidl)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/\$(IntDir)")

add_executable(VSMidl main.cpp test.idl)
