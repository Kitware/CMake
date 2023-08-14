
cmake_minimum_required(VERSION 3.18)
project(ISPCDynamicLibrary CXX ISPC)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_ISPC_FLAGS "--arch=x86")
endif()


add_library(ispc_objects1 STATIC extra.ispc extra.cxx)
add_library(ispc_objects2 SHARED simple.ispc)
target_sources(ispc_objects2 PRIVATE simple.cxx)

set_target_properties(ispc_objects1 PROPERTIES POSITION_INDEPENDENT_CODE ON)

if("${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
  set_property(TARGET ispc_objects1 PROPERTY ISPC_INSTRUCTION_SETS "neon-i32x4")
  set_property(TARGET ispc_objects2 PROPERTY ISPC_INSTRUCTION_SETS "neon-i32x4")
else()
  set_property(TARGET ispc_objects1 PROPERTY ISPC_INSTRUCTION_SETS "sse2-i32x4;avx1-i32x16;avx2-i32x4")
  set_property(TARGET ispc_objects2 PROPERTY ISPC_INSTRUCTION_SETS "sse2-i32x4")
endif()

target_link_libraries(ispc_objects2 PUBLIC ispc_objects1)

add_executable(ISPCDynamicLibrary main.cxx)
target_link_libraries(ISPCDynamicLibrary PUBLIC ispc_objects2)
