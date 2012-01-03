
#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
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

INCLUDE(CMakeTestCompilerCommon)

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected C++ compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
IF(NOT CMAKE_CXX_COMPILER_WORKS)
  PrintTestCompilerStatus("CXX" "")
  FILE(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCXXCompiler.cxx 
    "#ifndef __cplusplus\n"
    "# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"\n"
    "#endif\n"
    "int main(){return 0;}\n")
  TRY_COMPILE(CMAKE_CXX_COMPILER_WORKS ${CMAKE_BINARY_DIR} 
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCXXCompiler.cxx
    OUTPUT_VARIABLE __CMAKE_CXX_COMPILER_OUTPUT)
  SET(CXX_TEST_WAS_RUN 1)
ENDIF(NOT CMAKE_CXX_COMPILER_WORKS)

IF(NOT CMAKE_CXX_COMPILER_WORKS)
  PrintTestCompilerStatus("CXX" " -- broken")
  # if the compiler is broken make sure to remove the platform file
  # since Windows-cl configures both c/cxx files both need to be removed
  # when c or c++ fails
  FILE(REMOVE ${CMAKE_PLATFORM_ROOT_BIN}/CMakeCPlatform.cmake )
  FILE(REMOVE ${CMAKE_PLATFORM_ROOT_BIN}/CMakeCXXPlatform.cmake )
  FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the CXX compiler works failed with "
    "the following output:\n${__CMAKE_CXX_COMPILER_OUTPUT}\n\n")
  MESSAGE(FATAL_ERROR "The C++ compiler \"${CMAKE_CXX_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${__CMAKE_CXX_COMPILER_OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
ELSE(NOT CMAKE_CXX_COMPILER_WORKS)
  IF(CXX_TEST_WAS_RUN)
    PrintTestCompilerStatus("CXX" " -- works")
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the CXX compiler works passed with "
      "the following output:\n${__CMAKE_CXX_COMPILER_OUTPUT}\n\n")
  ENDIF(CXX_TEST_WAS_RUN)
  SET(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

  IF(CMAKE_CXX_COMPILER_FORCED)
    # The compiler configuration was forced by the user.
    # Assume the user has configured all compiler information.
  ELSE(CMAKE_CXX_COMPILER_FORCED)
    # Try to identify the ABI and configure it into CMakeCXXCompiler.cmake
    INCLUDE(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerABI.cmake)
    CMAKE_DETERMINE_COMPILER_ABI(CXX ${CMAKE_ROOT}/Modules/CMakeCXXCompilerABI.cpp)
    CONFIGURE_FILE(
      ${CMAKE_ROOT}/Modules/CMakeCXXCompiler.cmake.in
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXCompiler.cmake
      @ONLY IMMEDIATE # IMMEDIATE must be here for compatibility mode <= 2.0
      )
    INCLUDE(${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXCompiler.cmake)
  ENDIF(CMAKE_CXX_COMPILER_FORCED)
  IF(CMAKE_CXX_SIZEOF_DATA_PTR)
    FOREACH(f ${CMAKE_CXX_ABI_FILES})
      INCLUDE(${f})
    ENDFOREACH()
    UNSET(CMAKE_CXX_ABI_FILES)
  ENDIF()
ENDIF(NOT CMAKE_CXX_COMPILER_WORKS)

UNSET(__CMAKE_CXX_COMPILER_OUTPUT)
