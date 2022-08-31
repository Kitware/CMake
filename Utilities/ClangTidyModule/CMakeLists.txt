# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
cmake_minimum_required(VERSION 3.13)
project(CMakeClangTidyModule C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)

add_library(cmake-clang-tidy-module MODULE
  Module.cxx
  )
target_include_directories(cmake-clang-tidy-module PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(cmake-clang-tidy-module PRIVATE clang-tidy)
