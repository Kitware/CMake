cmake_minimum_required(VERSION 3.14)

project(WarnUnusedCliUnused)

# Remove UNUSED_CLI_VARIABLE from the cache to trigger the
# CMake warning message on re-builds as well.
unset(UNUSED_CLI_VARIABLE CACHE)

add_library(dummy empty.cpp)
