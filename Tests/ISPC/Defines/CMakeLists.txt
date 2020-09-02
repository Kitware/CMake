cmake_minimum_required(VERSION 3.18)
project(ISPCDefines CXX ISPC)

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
