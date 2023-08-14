# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
cmake_minimum_required(VERSION 3.13)
project(CMakeClangTidyModule C CXX)

get_filename_component(tmp "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
get_filename_component(CMake_SOURCE_DIR "${tmp}" PATH)

find_package(Clang REQUIRED)

if(LLVM_VERSION_MAJOR GREATER_EQUAL 16)
  set(CMAKE_CXX_STANDARD 17)
else()
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cmake-clang-tidy-module MODULE
  Module.cxx

  OstringstreamUseCmstrcatCheck.cxx
  OstringstreamUseCmstrcatCheck.h
  StringConcatenationUseCmstrcatCheck.cxx
  StringConcatenationUseCmstrcatCheck.h
  UseBespokeEnumClassCheck.cxx
  UseBespokeEnumClassCheck.h
  UseCmstrlenCheck.cxx
  UseCmstrlenCheck.h
  UseCmsysFstreamCheck.cxx
  UseCmsysFstreamCheck.h
  UsePragmaOnceCheck.cxx
  UsePragmaOnceCheck.h
  )
target_include_directories(cmake-clang-tidy-module PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(cmake-clang-tidy-module PRIVATE clang-tidy)

option(RUN_TESTS "Run the tests for the clang-tidy module" OFF)
if(RUN_TESTS)
  enable_testing()
  add_subdirectory(Tests)
endif()
