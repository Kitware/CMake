cmake_minimum_required(VERSION 3.10)
project(VSResourceNinjaForceRSP LANGUAGES C)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE TRUE)

add_library(ResourceLib lib.c test.rc)
add_executable(VSResourceNinjaForceRSP main.c test.rc)
