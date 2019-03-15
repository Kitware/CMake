cmake_minimum_required(VERSION 3.9)
project(${RunCMake_TEST} NONE)

# Tests assume no previous downloads in the output directory
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/_deps)

include(${RunCMake_TEST}.cmake)
