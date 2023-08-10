cmake_minimum_required(VERSION 3.18)
project(ispc_spaces_in_path ISPC CXX)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE ON)

# Make sure we can handle an arg file with tricky defines including spaces in -I include
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/path with spaces/simple_include.h"
"
  typedef float FLOAT_TYPE;
"
)

add_executable(ISPCResponseAndDefine main.cxx simple.ispc)
set_target_properties(ISPCResponseAndDefine PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(ISPCResponseAndDefine PRIVATE  "${CMAKE_CURRENT_BINARY_DIR}")

if("${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
  target_compile_options(ISPCResponseAndDefine PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--target=neon-i32x4>")
else()
  target_compile_options(ISPCResponseAndDefine PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--target=sse2-i32x4>")
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_options(ISPCResponseAndDefine PRIVATE "$<$<COMPILE_LANGUAGE:ISPC>:--arch=x86>")
  endif()
endif()

target_compile_definitions(ISPCResponseAndDefine PRIVATE
  "$<$<COMPILE_LANGUAGE:ISPC>:STRUCT_DEFINE=struct{uniform int a>;M_PI=3.14159f")
target_include_directories(ISPCResponseAndDefine PRIVATE
  "$<$<COMPILE_LANGUAGE:ISPC>:${CMAKE_CURRENT_BINARY_DIR}/fake path with spaces>"
  "$<$<COMPILE_LANGUAGE:ISPC>:${CMAKE_CURRENT_BINARY_DIR}/path with spaces>")
