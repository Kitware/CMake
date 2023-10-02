cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_deep_chain CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(CheckCompilerFlag)
  check_compiler_flag(CXX "-Wread-modules-implicitly" have_implicit_module_warning)
  if (have_implicit_module_warning)
    add_compile_options(-Werror=read-modules-implicitly)
  endif ()
endif ()

add_library(a STATIC)
target_sources(a
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        a.cxx)
target_compile_features(a PUBLIC cxx_std_20)

add_library(b STATIC)
target_sources(b
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        b.cxx)
target_compile_features(b PUBLIC cxx_std_20)
target_link_libraries(b PUBLIC a)

add_library(c STATIC)
target_sources(c
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        c.cxx)
target_compile_features(c PUBLIC cxx_std_20)
target_link_libraries(c PUBLIC b)

add_library(d STATIC)
target_sources(d
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        d.cxx)
target_compile_features(d PUBLIC cxx_std_20)
target_link_libraries(d PUBLIC c)

add_library(e STATIC)
target_sources(e
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        e.cxx)
target_compile_features(e PUBLIC cxx_std_20)
target_link_libraries(e PUBLIC d)

add_executable(exe)
target_link_libraries(exe PRIVATE e)
target_sources(exe
  PRIVATE
    main.cxx)

add_test(NAME exe COMMAND exe)
