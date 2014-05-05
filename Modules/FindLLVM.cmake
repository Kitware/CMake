# - Find LLVM include dirs and libraries
# The module defines the following variables:
#
#  LLVM_FOUND            - True if headers and requested libraries were found
#  LLVM_CONFIG_EXE       - Full path to llvm-config program
#  LLVM_CXX_FLAGS        - <llvm-config> --cxxflags
#  LLVM_C_FLAGS          - <llvm-config> --cflags
#  LLVM_LD_FLAGS         - <llvm-config> --ldflags
#  LLVM_INSTALL_PREFIX   - <llvm-config> --prefix
#  LLVM_VERSION          - <llvm-config> --version
#  LLVM_LIBS             - <llvm-config> --libs
#  LLVM_INCLUDE_DIR      - <llvm-config> --includedir
#
# The required version of LLVM can be specified using the
# standard CMake syntax, e.g. find_package(LLVM 3.2)
#
# This module may rely on the info from:
#  LLVM_ADDITIONAL_VERSIONS
#                        - List of LLVM versions not known to this module
#
# The module defines the macros:
#  get_llvm_config_var(<args> <output>)
# which will call llvm-config with the specified <args> and store the result into <output>.

#=============================================================================
# Copyright 2014 Roman Proskuryakov
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


include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CMakePushCheckState)
include(FindPackageHandleStandardArgs)

macro(get_llvm_config_var args out_var)
  execute_process(
    COMMAND ${LLVM_CONFIG_EXE} ${args}
    OUTPUT_VARIABLE ${out_var}
    RESULT_VARIABLE exit_code
    ERROR_VARIABLE std_err_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (exit_code)
    message(SEND_ERROR "Executing '${LLVM_CONFIG_EXE} ${args}' exited with '${exit_code}'")
    message(FATAL_ERROR "Error message: ${std_err_output}")
  endif()
endmacro()

macro(check_llvm_header header out_var)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_FLAGS "${LLVM_CXX_FLAGS}")
  CHECK_INCLUDE_FILE_CXX("${header}" ${out_var})
  cmake_pop_check_state()
endmacro()

macro(check_llvm_source_compiles code out_var)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_FLAGS "${LLVM_CXX_FLAGS} ${LLVM_LD_FLAGS}")
  set(CMAKE_REQUIRED_LIBRARIES ${LLVM_LIBS} dl pthread)
  CHECK_CXX_SOURCE_COMPILES("${code}" ${out_var})
  cmake_pop_check_state()
endmacro()

set(LLVM_CONFIG_NAMES "llvm-config-${LLVM_FIND_VERSION}" llvm-config)
foreach(version ${LLVM_ADDITIONAL_VERSIONS} 2.8 2.9 3.0 3.1 3.2 3.3 3.4 3.5)
  list(APPEND LLVM_CONFIG_NAMES "llvm-config-${version}")
endforeach()

find_program(LLVM_CONFIG_EXE NAMES ${LLVM_CONFIG_NAMES} DOC "Full path to llvm-config")
unset(LLVM_CONFIG_NAMES)

if (LLVM_CONFIG_EXE)
  get_llvm_config_var(--cxxflags   LLVM_CXX_FLAGS)
  get_llvm_config_var(--cflags     LLVM_C_FLAGS)
  get_llvm_config_var(--ldflags    LLVM_LD_FLAGS)
  get_llvm_config_var(--prefix     LLVM_INSTALL_PREFIX)
  get_llvm_config_var(--version    LLVM_VERSION)
  get_llvm_config_var(--libs       LLVM_LIBS)
  get_llvm_config_var(--includedir LLVM_INCLUDE_DIR)

  # Header 'Pass.h' locates in 'include/llvm/' directory since 1.9 till 3.4 release
  check_llvm_header("llvm/Pass.h" LLVM_PASS_H)
  check_llvm_source_compiles("#include <llvm/LinkAllPasses.h> \n int main(){ return 0; }" LLVM_PASSES_LINKED)

  if (NOT LLVM_PASS_H OR NOT LLVM_PASSES_LINKED)
    message(STATUS "See ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log for details")
  endif()
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS( LLVM
  REQUIRED_VARS LLVM_CONFIG_EXE LLVM_PASS_H LLVM_PASSES_LINKED
  VERSION_VAR LLVM_VERSION
)

mark_as_advanced(LLVM_CONFIG_EXE)
