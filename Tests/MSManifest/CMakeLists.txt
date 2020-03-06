cmake_minimum_required(VERSION 3.3)
project(MSManifest C)

include(CTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(Subdir)
add_subdirectory(Subdir2)
