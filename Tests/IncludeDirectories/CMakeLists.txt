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

# default to testing with full path
# some compilers can not handle the escape for directories
# with spaces in them.
set(USE_FULLPATH TRUE)
if(WATCOM OR MSVC60)
  set(USE_FULLPATH FALSE)
endif()
if(USE_FULLPATH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \"-I${CMAKE_BINARY_DIR}/Flags\"")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -IFlags")
endif()

include_directories(${CMAKE_BINARY_DIR}/IncDir)
if(USE_FULLPATH)
  set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
    "\"-I${CMAKE_BINARY_DIR}/SrcProp\"")
else()
  set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
    "-ISrcProp")
endif()

add_executable(IncludeDirectories main.cpp)

if(USE_FULLPATH)
  set_target_properties(IncludeDirectories
    PROPERTIES COMPILE_FLAGS "\"-I${CMAKE_BINARY_DIR}/TarProp\"")
else()
  set_target_properties(IncludeDirectories
    PROPERTIES COMPILE_FLAGS "-ITarProp")
endif()

add_subdirectory(TargetIncludeDirectories)
if ((CMAKE_C_COMPILER_ID STREQUAL GNU AND CMAKE_C_COMPILER_VERSION VERSION_GREATER 3)
    OR CMAKE_C_COMPILER_ID STREQUAL Clang)
  add_subdirectory(SystemIncludeDirectories)
endif()

set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}")
get_property(propContent DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
if (NOT propContent STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "Setting DIRECTORY property failed.")
endif()
set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
get_property(propContentAfter DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
if (NOT propContentAfter STREQUAL "")
  message(SEND_ERROR "Clearing DIRECTORY property failed.")
endif()

add_library(empty_entry_test SHARED empty.cpp)
set_target_properties(empty_entry_test PROPERTIES INCLUDE_DIRECTORIES "")
include_directories(/one/two
  " "
  "  "
)
get_target_property(incs empty_entry_test INCLUDE_DIRECTORIES)
if (NOT incs STREQUAL ";/one/two")
  message(SEND_ERROR "Empty include_directories entry was not ignored.")
endif()
