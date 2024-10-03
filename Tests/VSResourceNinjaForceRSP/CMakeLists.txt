cmake_minimum_required(VERSION 3.10)
project(VSResourceNinjaForceRSP)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE TRUE)

add_library(ResourceLib lib.cpp test.rc)
add_executable(VSResourceNinjaForceRSP main.cpp test.rc)
