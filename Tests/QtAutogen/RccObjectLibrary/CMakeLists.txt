cmake_minimum_required(VERSION 3.16)
project(RccObjectLibrary)
include("../AutogenCoreTest.cmake")

# Test for issue #26977:
# AUTOGEN_BETTER_GRAPH_MULTI_CONFIG with AUTORCC and OBJECT libraries.

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# OBJECT library with a .qrc file
add_library(rcc_obj OBJECT
  rcc_obj.cpp
  rcc_obj.qrc
)
target_link_libraries(rcc_obj PRIVATE ${QT_LIBRARIES})

# Executable that uses the OBJECT library
add_executable(rccObjectLibrary
  main.cpp
  $<TARGET_OBJECTS:rcc_obj>
)
target_link_libraries(rccObjectLibrary PRIVATE ${QT_LIBRARIES})
