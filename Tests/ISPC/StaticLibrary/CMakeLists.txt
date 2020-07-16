
cmake_minimum_required(VERSION 3.18)
project(ISPCStaticLibrary CXX ISPC)

add_library(ispc_objects STATIC simple.ispc)

target_compile_options(ispc_objects PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--target=sse2-i32x4;--arch=x86-64>")
target_include_directories(ispc_objects INTERFACE  "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(ispc_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(ISPCStaticLibrary main.cxx)
target_link_libraries(ISPCStaticLibrary PRIVATE ispc_objects)
