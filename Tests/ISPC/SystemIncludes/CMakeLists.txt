cmake_minimum_required(VERSION 3.18)
project(ispc_spaces_in_path ISPC CXX)


add_executable(ISPCSystemIncludes main.cxx simple.ispc)
set_target_properties(ISPCSystemIncludes PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ISPCSystemIncludes PROPERTIES ISPC_HEADER_SUFFIX ".ispc.h")
target_include_directories(ISPCSystemIncludes SYSTEM PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if("${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
  target_compile_options(ISPCSystemIncludes PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--target=neon-i32x4>")
else()
  target_compile_options(ISPCSystemIncludes PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--target=sse2-i32x4>")
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_options(ISPCSystemIncludes PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--arch=x86>")
  endif()
endif()
