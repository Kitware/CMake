cmake_minimum_required(VERSION 3.24)
project(SwiftMixLib C CXX Swift)

add_library(SwiftMixedLib lib.c lib.cpp lib.swift)
add_executable(Swifty main.swift)
target_link_libraries(Swifty PUBLIC SwiftMixedLib)
