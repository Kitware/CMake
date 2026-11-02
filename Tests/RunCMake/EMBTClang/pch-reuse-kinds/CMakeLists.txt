cmake_minimum_required(VERSION 3.23)
project(embt_pch_reuse CXX)

add_library(pch STATIC lib.cpp)
target_precompile_headers(pch PRIVATE pch.h)

add_library(dll SHARED dll.cpp)
add_library(mod MODULE dll.cpp)
add_executable(console console.cpp)
add_executable(gui WIN32 gui.cpp)
foreach(t IN ITEMS dll mod console gui)
  target_precompile_headers(${t} REUSE_FROM pch)
endforeach()
