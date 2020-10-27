cmake_minimum_required(VERSION 3.18)
project(ISPCObjectGenex CXX ISPC)

set(CMAKE_ISPC_INSTRUCTION_SETS "sse2-i32x4;sse4-i16x8;avx1-i32x16;avx2-i32x4;avx512knl-i32x16;avx512skx-i32x8")

add_library(ispc_objects OBJECT
  simple.ispc
  )
target_compile_definitions(ispc_objects PRIVATE
  $<$<COMPILE_LANG_AND_ID:ISPC,Intel>:M_PI=3.1415926535f>
)
set_target_properties(ispc_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set_source_files_properties(simple.ispc PROPERTIES COMPILE_OPTIONS "--arch=x86")
endif()


#Test ObjectFiles with file(GENERATE)
file(GENERATE
     OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gen_$<LOWER_CASE:$<CONFIG>/>path_to_objs.h
     CONTENT [[
#ifndef path_to_objs
#define path_to_objs

#include <string>

static std::string obj_paths = "$<TARGET_OBJECTS:ispc_objects>";

#endif

]]
)


add_executable(ISPCObjectGenex main.cxx)
add_dependencies(ISPCObjectGenex ispc_objects)

list(LENGTH CMAKE_ISPC_INSTRUCTION_SETS numberOfTargets)
math(EXPR numberOfTargets "${numberOfTargets}+1")
target_compile_definitions(ISPCObjectGenex PRIVATE
    "ExpectedISPCObjects=${numberOfTargets}"
    "CONFIG_TYPE=gen_$<LOWER_CASE:$<CONFIG>>"
    )
target_include_directories(ISPCObjectGenex PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )
target_compile_features(ISPCObjectGenex PRIVATE cxx_std_11)
