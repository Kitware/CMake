cmake_minimum_required(VERSION 2.8.12)
project(${RunCMake_TEST} NONE)

message(STATUS "source: '${CMAKE_SOURCE_DIR}'")
message(STATUS "binary: '${CMAKE_BINARY_DIR}'")
get_filename_component(real_source "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(real_binary "${CMAKE_BINARY_DIR}" REALPATH)
message(STATUS "real source: '${real_source}'")
message(STATUS "real binary: '${real_binary}'")
