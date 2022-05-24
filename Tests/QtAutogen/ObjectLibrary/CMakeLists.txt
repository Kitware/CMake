cmake_minimum_required(VERSION 3.16)
project(ObjectLibrary)
include("../AutogenCoreTest.cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Object library a defined in a subdirectory
add_subdirectory(a)

# Object library b defined locally
include_directories(b)
add_library(b OBJECT b/classb.cpp)
target_link_libraries(b PRIVATE ${QT_LIBRARIES})

# Executable with OBJECT library generator expressions
add_executable(someProgram main.cpp $<TARGET_OBJECTS:a> $<TARGET_OBJECTS:b>)
target_link_libraries(someProgram ${QT_LIBRARIES})

# Executable without its own AUTOMOC.
add_executable(someProgram2 main.cpp)
target_link_libraries(someProgram2 PRIVATE a b ${QT_LIBRARIES})
set_property(TARGET someProgram2 PROPERTY AUTOMOC OFF)
