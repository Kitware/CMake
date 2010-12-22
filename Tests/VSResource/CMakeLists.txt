cmake_minimum_required (VERSION 2.6)
project (VSResource)
add_definitions(/DCMAKE_RCDEFINE="test.txt")
string(REPLACE "/INCREMENTAL:YES" ""
  CMAKE_EXE_LINKER_FLAGS_DEBUG
  "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
add_executable(VSResource main.cpp test.rc)
