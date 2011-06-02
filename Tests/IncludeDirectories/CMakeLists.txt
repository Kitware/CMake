cmake_minimum_required (VERSION 2.6)
project(IncludeDirectories)

set(CMAKE_CXX_FLAGS "\"-I${CMAKE_SOURCE_DIR}/Flags\"")
include_directories(${CMAKE_SOURCE_DIR}/IncDir)
set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
  "\"-I${CMAKE_SOURCE_DIR}/SrcProp\"")
add_executable(IncludeDirectories main.cpp)
set_target_properties(IncludeDirectories
  PROPERTIES COMPILE_FLAGS "\"-I${CMAKE_SOURCE_DIR}/TarProp\"")
