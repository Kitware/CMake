cmake_minimum_required(VERSION 3.24)
project(cxx_modules_try_compile CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
try_compile(can_use_modules
  SOURCES_TYPE CXX_MODULE
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/importable.cxx"
  SOURCES_TYPE NORMAL
  SOURCE_FROM_FILE
    use_importable.cxx "${CMAKE_CURRENT_LIST_DIR}/use_importable.cxx"
  CXX_STANDARD 20)

if (NOT can_use_modules)
  message(FATAL_ERROR
    "`try_compile` could not compile sources using modules.")
endif ()
