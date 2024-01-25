cmake_minimum_required(VERSION 3.24)
project(SwiftMixLib C CXX Swift)

# Swift on Windows only provides a release runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

add_library(SwiftMixedLib lib.c lib.cpp lib.swift)
add_executable(Swifty main.swift)
target_link_libraries(Swifty PUBLIC SwiftMixedLib)

add_executable(c_main main.c)
target_link_libraries(c_main PUBLIC SwiftMixedLib)
