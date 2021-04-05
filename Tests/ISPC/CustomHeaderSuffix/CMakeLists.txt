
cmake_minimum_required(VERSION 3.18)
project(ISPCCustomHeaderSuffix CXX ISPC)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_ISPC_FLAGS "--arch=x86")
endif()

set(CMAKE_ISPC_INSTRUCTION_SETS "sse2-i32x4;sse4-i8x16")

set(CMAKE_ISPC_HEADER_SUFFIX ".ispc.h")

add_library(ispc_suffix_1 OBJECT simple.ispc)
add_library(ispc_suffix_2 OBJECT extra.ispc)

set_target_properties(ispc_suffix_2 PROPERTIES ISPC_HEADER_SUFFIX "___ispc.h")

set_target_properties(ispc_suffix_1 ispc_suffix_2
  PROPERTIES POSITION_INDEPENDENT_CODE ON
)

add_executable(ISPCCustomHeaderSuffix main.cxx extra.cxx)
target_link_libraries(ISPCCustomHeaderSuffix PRIVATE ispc_suffix_1 ispc_suffix_2)
