cmake_minimum_required(VERSION 2.8.12)
enable_language(CXX)

add_library(foo empty.cpp)

add_library(invalid$name ALIAS foo)
