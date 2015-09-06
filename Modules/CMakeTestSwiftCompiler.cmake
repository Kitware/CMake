
#=============================================================================
# Copyright 2003-2015 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if(CMAKE_Swift_COMPILER_FORCED)
  # The compiler configuration was forced by the user.
  # Assume the user has configured all compiler information.
  set(CMAKE_Swift_COMPILER_WORKS TRUE)
  return()
endif()

include(CMakeTestCompilerCommon)

# Remove any cached result from an older CMake version.
# We now store this in CMakeSwiftCompiler.cmake.
unset(CMAKE_Swift_COMPILER_WORKS CACHE)

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected C++ compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
if(NOT CMAKE_Swift_COMPILER_WORKS)
  PrintTestCompilerStatus("Swift" "")
  file(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/main.swift
    "import Foundation\n"
    "print(\"CMake\")\n")
  try_compile(CMAKE_Swift_COMPILER_WORKS ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/main.swift
    OUTPUT_VARIABLE __CMAKE_Swift_COMPILER_OUTPUT)
  # Move result from cache to normal variable.
  set(CMAKE_Swift_COMPILER_WORKS ${CMAKE_Swift_COMPILER_WORKS})
  unset(CMAKE_Swift_COMPILER_WORKS CACHE)
  set(Swift_TEST_WAS_RUN 1)
endif()

if(NOT CMAKE_Swift_COMPILER_WORKS)
  PrintTestCompilerStatus("Swift" " -- broken")
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the Swift compiler works failed with "
    "the following output:\n${__CMAKE_Swift_COMPILER_OUTPUT}\n\n")
  message(FATAL_ERROR "The Swift compiler \"${CMAKE_Swift_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${__CMAKE_Swift_COMPILER_OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
else()
  if(Swift_TEST_WAS_RUN)
    PrintTestCompilerStatus("Swift" " -- works")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the Swift compiler works passed with "
      "the following output:\n${__CMAKE_Swift_COMPILER_OUTPUT}\n\n")
  endif()
endif()

unset(__CMAKE_Swift_COMPILER_OUTPUT)
