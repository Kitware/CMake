
cmake_minimum_required(VERSION 3.18)
project(ISPCObjectLibrary CXX ISPC)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE ON)

add_library(ispc_objects OBJECT simple.ispc)

target_compile_options(ispc_objects PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--target=sse2-i32x4;--arch=x86-64>")
target_include_directories(ispc_objects INTERFACE  "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(ispc_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(ISPCObjectLibrary main.cxx)
target_link_libraries(ISPCObjectLibrary PRIVATE ispc_objects)
