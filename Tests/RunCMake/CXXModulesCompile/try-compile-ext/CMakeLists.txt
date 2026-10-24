cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_try_compile_ext CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

# The module sources reside outside of the project's source directory.
set(srcdir "${CMAKE_CURRENT_LIST_DIR}/../try-compile")

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
try_compile(can_use_modules_sources
  SOURCES_TYPE CXX_MODULE
  SOURCES
    "${srcdir}/importable.cxx"
  SOURCES_TYPE NORMAL
  SOURCES
    "${srcdir}/use_importable.cxx"
  CXX_STANDARD 20)

if (NOT can_use_modules_sources)
  message(FATAL_ERROR
    "`try_compile` could not compile module sources given by `SOURCES`.")
endif ()

try_compile(can_use_modules_copied
  SOURCES_TYPE CXX_MODULE
  SOURCE_FROM_FILE
    importable.cxx "${srcdir}/importable.cxx"
  SOURCES_TYPE NORMAL
  SOURCE_FROM_FILE
    use_importable.cxx "${srcdir}/use_importable.cxx"
  CXX_STANDARD 20)

if (NOT can_use_modules_copied)
  message(FATAL_ERROR
    "`try_compile` could not compile module sources given by "
    "`SOURCE_FROM_FILE`.")
endif ()
