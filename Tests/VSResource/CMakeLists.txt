cmake_minimum_required(VERSION 2.8.3.20110118)
project (VSResource)

string(REPLACE "/INCREMENTAL:YES" ""
  CMAKE_EXE_LINKER_FLAGS_DEBUG
  "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.txt
  "${CMAKE_CURRENT_BINARY_DIR}/test with spaces.txt" COPYONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(/DCMAKE_RCDEFINE="test with spaces.txt")

add_executable(VSResource main.cpp test.rc)
