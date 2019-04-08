# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)
include(ExternalProject)

ExternalProject_Add(another_project
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/empty
  BINARY_DIR empty_build
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  )
