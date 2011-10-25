cmake_minimum_required(VERSION 2.8)

project(src)

include(CheckCXXCompilerFlag)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(lib)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
  "${CMAKE_CURRENT_BINARY_DIR}/lib"
)


add_executable(exec
  "main.cpp"
)

target_link_libraries(exec libB )
