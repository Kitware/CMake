
cmake_minimum_required(VERSION 3.18)
project(ISPCChainedStaticLibraries CXX ISPC)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_ISPC_FLAGS "--arch=x86")
endif()


add_library(ispc_objects1 STATIC extra.ispc extra.cxx)
add_library(ispc_objects2 STATIC simple.ispc)

set_target_properties(ispc_objects1 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ispc_objects1 PROPERTIES ISPC_INSTRUCTION_SETS "sse2-i32x4;avx1-i32x16;avx2-i32x4")

set_target_properties(ispc_objects2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ispc_objects2 PROPERTIES ISPC_INSTRUCTION_SETS "sse2-i32x4")

target_link_libraries(ispc_objects2 PRIVATE ispc_objects1)

add_executable(ISPCChainedStaticLibraries main.cxx)
target_link_libraries(ISPCChainedStaticLibraries PUBLIC ispc_objects2)
