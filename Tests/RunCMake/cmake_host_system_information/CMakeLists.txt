cmake_minimum_required(VERSION 3.21)

project(${RunCMake_TEST} NONE)

set(CMAKE_SYSROOT ${PROJECT_SOURCE_DIR}/${RunCMake_TEST})

include(${RunCMake_TEST}.cmake)
