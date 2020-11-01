# Used to verify that the values match what is passed via -S and -B, and are retained in cache.
message("PreLoad.cmake: CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("PreLoad.cmake: CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

set(PRELOAD_BINARY_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "value of cmake_binary_dir during preload")
set(PRELOAD_SOURCE_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH "value of cmake_source_dir during preload")
