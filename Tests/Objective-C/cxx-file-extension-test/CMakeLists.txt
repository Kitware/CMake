cmake_minimum_required(VERSION 3.15)

project(cxx-file-extension-test)

string(APPEND CMAKE_CXX_FLAGS " -std=c++11")
set(CMAKE_CXX_STANDARD 14)

add_executable(cxx-file-extension-test main.m)
