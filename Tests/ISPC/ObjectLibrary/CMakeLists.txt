
cmake_minimum_required(VERSION 3.18)
project(ISPCObjectLibrary CXX ISPC)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE ON)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_ISPC_FLAGS "--arch=x86")
endif()


add_library(ispc_objects OBJECT simple.ispc subdir/extra.ispc)

set_target_properties(ispc_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ispc_objects PROPERTIES ISPC_INSTRUCTION_SETS "sse2-i32x4;sse4-i8x16")


add_executable(ISPCObjectLibrary main.cxx extra.cxx)
target_link_libraries(ISPCObjectLibrary PRIVATE ispc_objects)
