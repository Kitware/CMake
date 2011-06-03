cmake_minimum_required (VERSION 2.6)
project(IncludeDirectories)

file(WRITE ${CMAKE_BINARY_DIR}/Flags/Flags.h
"//Flags.h
")
file(WRITE ${CMAKE_BINARY_DIR}/IncDir/IncDir.h
"//IncDir.h
")
file(WRITE ${CMAKE_BINARY_DIR}/SrcProp/SrcProp.h
"//SrcProp.h
")
file(WRITE ${CMAKE_BINARY_DIR}/TarProp/TarProp.h
"//TarProp.h
")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -IFlags")
include_directories(${CMAKE_BINARY_DIR}/IncDir)
set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
  "-ISrcProp")
add_executable(IncludeDirectories main.cpp)
set_target_properties(IncludeDirectories
  PROPERTIES COMPILE_FLAGS "-ITarProp")
