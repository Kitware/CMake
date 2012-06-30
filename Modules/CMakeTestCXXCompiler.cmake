
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

include(CMakeTestCompilerCommon)

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected C++ compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
if(NOT CMAKE_CXX_COMPILER_WORKS)
  PrintTestCompilerStatus("CXX" "")
  file(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCXXCompiler.cxx
    "#ifndef __cplusplus\n"
    "# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"\n"
    "#endif\n"
    "int main(){return 0;}\n")
  try_compile(CMAKE_CXX_COMPILER_WORKS ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCXXCompiler.cxx
    OUTPUT_VARIABLE __CMAKE_CXX_COMPILER_OUTPUT)
  set(CXX_TEST_WAS_RUN 1)
endif(NOT CMAKE_CXX_COMPILER_WORKS)

if(NOT CMAKE_CXX_COMPILER_WORKS)
  PrintTestCompilerStatus("CXX" " -- broken")
  # if the compiler is broken make sure to remove the platform file
  # since Windows-cl configures both c/cxx files both need to be removed
  # when c or c++ fails
  file(REMOVE ${CMAKE_PLATFORM_ROOT_BIN}/CMakeCPlatform.cmake )
  file(REMOVE ${CMAKE_PLATFORM_ROOT_BIN}/CMakeCXXPlatform.cmake )
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the CXX compiler works failed with "
    "the following output:\n${__CMAKE_CXX_COMPILER_OUTPUT}\n\n")
  message(FATAL_ERROR "The C++ compiler \"${CMAKE_CXX_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${__CMAKE_CXX_COMPILER_OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
else(NOT CMAKE_CXX_COMPILER_WORKS)
  if(CXX_TEST_WAS_RUN)
    PrintTestCompilerStatus("CXX" " -- works")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the CXX compiler works passed with "
      "the following output:\n${__CMAKE_CXX_COMPILER_OUTPUT}\n\n")
  endif(CXX_TEST_WAS_RUN)
  set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

  if(CMAKE_CXX_COMPILER_FORCED)
    # The compiler configuration was forced by the user.
    # Assume the user has configured all compiler information.
  else(CMAKE_CXX_COMPILER_FORCED)
    # Try to identify the ABI and configure it into CMakeCXXCompiler.cmake
    include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerABI.cmake)
    CMAKE_DETERMINE_COMPILER_ABI(CXX ${CMAKE_ROOT}/Modules/CMakeCXXCompilerABI.cpp)
    configure_file(
      ${CMAKE_ROOT}/Modules/CMakeCXXCompiler.cmake.in
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXCompiler.cmake
      @ONLY IMMEDIATE # IMMEDIATE must be here for compatibility mode <= 2.0
      )
    include(${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXCompiler.cmake)
  endif(CMAKE_CXX_COMPILER_FORCED)
  if(CMAKE_CXX_SIZEOF_DATA_PTR)
    foreach(f ${CMAKE_CXX_ABI_FILES})
      include(${f})
    endforeach()
    unset(CMAKE_CXX_ABI_FILES)
  endif()
endif(NOT CMAKE_CXX_COMPILER_WORKS)

unset(__CMAKE_CXX_COMPILER_OUTPUT)
