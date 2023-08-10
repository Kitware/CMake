cmake_minimum_required(VERSION 3.18)
project(ISPCDefines CXX ISPC)

if("${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
  set(CMAKE_ISPC_INSTRUCTION_SETS "neon-i32x4")
else()
  set(CMAKE_ISPC_INSTRUCTION_SETS "sse2-i32x4;sse4-i16x8;avx1-i32x16;avx2-i32x4;avx512knl-i32x16;avx512skx-i32x8")
endif()

set(CMAKE_ISPC_FLAGS -DM_PI=3.1415926535f)
add_compile_definitions([==[STRUCT_DEFINE=struct{uniform int a]==])

add_executable(ISPCDefines
  main.cxx
  simple.ispc
  )

set_target_properties(ISPCDefines PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set_source_files_properties(simple.ispc PROPERTIES COMPILE_OPTIONS "--arch=x86")
endif()
