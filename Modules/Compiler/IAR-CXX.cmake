# This file is processed when the IAR C++ Compiler is used
#
# C++ Language Specification support
#  - Newer versions of the IAR C++ Compiler require the --c++ flag to build a C++ file.
#    Earlier versions for non-ARM architectures provided Embedded C++, enabled with the --eec++ flag.
#
# The IAR Language Extensions
#  - The IAR Language Extensions can be enabled by -e flag
#
include(Compiler/IAR)
include(Compiler/CMakeCommonCompilerMacros)

if(NOT CMAKE_CXX_COMPILER_VERSION)
  message(FATAL_ERROR "Could not detect CMAKE_CXX_COMPILER_VERSION. This should be automatic. Check your product license.\n")
endif()

# The CMAKE_IAR_CXX_FLAG variable can be used in your toolchain file
# for overriding the default flag for compiling C++ source files.
if(NOT CMAKE_IAR_CXX_FLAG)
  if((NOT CMAKE_CXX_STANDARD_COMPUTED_DEFAULT EQUAL 98) OR
     ("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM"))
    set(CMAKE_IAR_CXX_FLAG --c++)
  else()
    set(CMAKE_IAR_CXX_FLAG --eec++)
  endif()
endif()

set(CMAKE_CXX_STANDARD_COMPILE_OPTION "")
set(CMAKE_CXX_EXTENSION_COMPILE_OPTION -e) # Unused after CMP0128

# Stubs for standard/extended compiler options
if(NOT CMAKE_CXX_STANDARD_COMPUTED_DEFAULT EQUAL 98)
  if(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT GREATER_EQUAL 20)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION --libc++)
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION --libc++ -e)
  endif()
  if(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT GREATER_EQUAL 17)
    set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "")
    set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION -e)
  endif()
  if(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT GREATER_EQUAL 14)
    set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "")
    set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION -e)
  endif()
  if(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT GREATER_EQUAL 11)
    set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "")
    set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION -e)
  endif()
  if(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT EQUAL 03)
    set(CMAKE_CXX03_STANDARD_COMPILE_OPTION "")
    set(CMAKE_CXX03_EXTENSION_COMPILE_OPTION -e)
    set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
    set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION -e)
  endif()
else()
  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION -e)
endif()

# Architecture specific
if("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
    # IAR C++ Compiler for Arm prior version 5.xx uses XLINK. Support in CMake is not implemented.
    message(FATAL_ERROR "IAR C++ Compiler for Arm version ${CMAKE_CXX_COMPILER_VERSION} not supported by CMake.")
  endif()
  __compiler_iar_ilink(CXX)
  __compiler_check_default_language_standard(CXX 5.10 98 8.10 14 8.40 17 10.10 20)

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "RX")
  __compiler_iar_ilink(CXX)
  __compiler_check_default_language_standard(CXX 2.10 98 4.10 14 4.20 17)

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "RH850")
  __compiler_iar_ilink(CXX)
  __compiler_check_default_language_standard(CXX 1.10 98 2.10 14 2.21 17)

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "RL78")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 2)
    # IAR C++ Compiler for RL78 prior version 2.xx uses XLINK. Support in CMake is not implemented.
    message(FATAL_ERROR "IAR C++ Compiler for RL78 version ${CMAKE_CXX_COMPILER_VERSION} not supported by CMake.")
  endif()
  __compiler_iar_ilink(CXX)
  __compiler_check_default_language_standard(CXX 2.10 98 4.10 14 4.20 17)

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "RISCV")
  __compiler_iar_ilink(CXX)
  __compiler_check_default_language_standard(CXX 1.10 98 1.10 14 1.21 17)

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "AVR")
  __compiler_iar_xlink(CXX)
  __compiler_check_default_language_standard(CXX 7.10 98 8.10 17)
  set(CMAKE_CXX_OUTPUT_EXTENSION ".r90")

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "MSP430")
  __compiler_iar_xlink(CXX)
  __compiler_check_default_language_standard(CXX 5.10 98)
  set(CMAKE_CXX_OUTPUT_EXTENSION ".r43")

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "V850")
  __compiler_iar_xlink(CXX)
  __compiler_check_default_language_standard(CXX 1.10 98)
  set(CMAKE_CXX_OUTPUT_EXTENSION ".r85")

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "8051")
  __compiler_iar_xlink(CXX)
  __compiler_check_default_language_standard(CXX 6.10 98)
  set(CMAKE_CXX_OUTPUT_EXTENSION ".r51")

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "STM8")
  __compiler_iar_ilink(CXX)
  __compiler_check_default_language_standard(CXX 3.11 98)

else()
  message(FATAL_ERROR "CMAKE_CXX_COMPILER_ARCHITECTURE_ID not detected. This should be automatic." )
endif()
