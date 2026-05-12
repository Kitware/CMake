# This file is processed when the IAR C Compiler is used.

include(Compiler/IAR)
include(Compiler/CMakeCommonCompilerMacros)

if(NOT CMAKE_C_COMPILER_VERSION)
  message(FATAL_ERROR "Could not detect CMAKE_C_COMPILER_VERSION. This should be automatic. Check your product license.\n")
endif()

# Unused after CMP0128
set(CMAKE_C_EXTENSION_COMPILE_OPTION -e)

# Stubs for standard/extended compile options
if((NOT CMAKE_C_STANDARD_COMPUTED_DEFAULT EQUAL 99) AND
   (NOT CMAKE_C_STANDARD_COMPUTED_DEFAULT EQUAL 90))
  if(CMAKE_C_STANDARD_COMPUTED_DEFAULT GREATER_EQUAL 17)
    set(CMAKE_C17_STANDARD_COMPILE_OPTION "")
    set(CMAKE_C17_EXTENSION_COMPILE_OPTION -e)
  endif()
  if(CMAKE_C_STANDARD_COMPUTED_DEFAULT GREATER_EQUAL 11)
    set(CMAKE_C11_STANDARD_COMPILE_OPTION "")
    set(CMAKE_C11_EXTENSION_COMPILE_OPTION -e)
  endif()
  set(CMAKE_C99_STANDARD_COMPILE_OPTION "")
  set(CMAKE_C99_EXTENSION_COMPILE_OPTION -e)
elseif(CMAKE_C_STANDARD_COMPUTED_DEFAULT EQUAL 99)
  set(CMAKE_C99_STANDARD_COMPILE_OPTION "")
  set(CMAKE_C99_EXTENSION_COMPILE_OPTION -e)
endif()

# C90 Mode
#  - IAR Compilers with *internal* version >= v8.0.0 require the --c89 flag.
#  - IAR Compilers with *internal* version < v8.0.0 has C90 mode by default.
if(CMAKE_C_COMPILER_VERSION_INTERNAL GREATER_EQUAL 0x00080000)
  set(CMAKE_C90_STANDARD_COMPILE_OPTION --c89)
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION --c89 -e)
elseif(CMAKE_C_COMPILER_VERSION_INTERNAL LESS 0x00080000)
  set(CMAKE_C90_STANDARD_COMPILE_OPTION "")
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION -e)
endif()


# Architecture specific
if("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM")
  if (CMAKE_C_COMPILER_VERSION VERSION_LESS 5)
    # IAR C Compiler for Arm prior version 5.xx uses XLINK. Support in CMake is not implemented.
    message(FATAL_ERROR "IAR C Compiler for Arm version ${CMAKE_C_COMPILER_VERSION} not supported by CMake.")
  endif()
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 5.10 90 6.10 99 8.10 11 8.40 17)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "RX")
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 1.10 90 2.10 99 4.10 11 4.20 17)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "RH850")
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 1.10 90 1.10 99 2.10 11 2.21 17)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "RL78")
  if(CMAKE_C_COMPILER_VERSION VERSION_LESS 2)
    # IAR C Compiler for RL78 prior version 2.xx uses XLINK. Support in CMake is not implemented.
    message(FATAL_ERROR "IAR C Compiler for RL78 version ${CMAKE_C_COMPILER_VERSION} not supported by CMake.")
  endif()
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 2.10 90 2.10 99 4.10 11 4.20 17)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "RISCV")
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 1.10 90 1.10 99 1.10 11 1.21 17)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "AVR")
  __compiler_iar_xlink(C)
  __compiler_check_default_language_standard(C 7.10 99 8.10 17)
  set(CMAKE_C_OUTPUT_EXTENSION ".r90")

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "MSP430")
  __compiler_iar_xlink(C)
  __compiler_check_default_language_standard(C 1.10 90 5.10 99)
  set(CMAKE_C_OUTPUT_EXTENSION ".r43")

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "V850")
  __compiler_iar_xlink(C)
  __compiler_check_default_language_standard(C 1.10 90 4.10 99)
  set(CMAKE_C_OUTPUT_EXTENSION ".r85")

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "8051")
  __compiler_iar_xlink(C)
  __compiler_check_default_language_standard(C 6.10 90 8.10 99)
  set(CMAKE_C_OUTPUT_EXTENSION ".r51")

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "STM8")
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 3.11 90 3.11 99)

else()
  message(FATAL_ERROR "CMAKE_C_COMPILER_ARCHITECTURE_ID not detected. This should be automatic.")
endif()
