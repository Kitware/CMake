cmake_minimum_required(VERSION 3.18)
project(ISPCDefines CXX ISPC)

set(CMAKE_ISPC_FLAGS -DM_PI=3.1415926535f [==[-DSTRUCT_DEFINE=struct{uniform int a]==])

add_executable(ISPCResponseFile
  main.cxx
  simple.ispc
  )
set_target_properties(ISPCResponseFile PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(ISPCResponseFile PRIVATE  "${CMAKE_CURRENT_BINARY_DIR}")
