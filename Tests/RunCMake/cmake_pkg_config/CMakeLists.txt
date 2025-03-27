cmake_minimum_required(VERSION 3.30)
project(${RunCMake_TEST} NONE)

set(CMAKE_PKG_CONFIG_PC_LIB_DIRS ${CMAKE_CURRENT_LIST_DIR}/PackageRoot)
include(${RunCMake_TEST}.cmake)
