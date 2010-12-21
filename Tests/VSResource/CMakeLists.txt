cmake_minimum_required (VERSION 2.6)
project (VSResource)
add_definitions(/DCMAKE_RCDEFINE="test.txt")
add_executable(VSResource main.cpp test.rc)
