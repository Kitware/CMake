# This file is processed when the IAR compiler is used for a C file

include(Compiler/IAR)
include(Compiler/CMakeCommonCompilerMacros)

# Common
if(NOT CMAKE_C_COMPILER_VERSION)
  message(FATAL_ERROR "CMAKE_C_COMPILER_VERSION not detected.  This should be automatic.")
endif()

set(CMAKE_C_EXTENSION_COMPILE_OPTION -e)

if(CMAKE_C_COMPILER_VERSION_INTERNAL VERSION_GREATER 7)
  set(CMAKE_C90_STANDARD_COMPILE_OPTION --c89)
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION --c89 -e)
  set(CMAKE_C99_STANDARD_COMPILE_OPTION "")
  set(CMAKE_C99_EXTENSION_COMPILE_OPTION -e)
elseif()
  set(CMAKE_C90_STANDARD_COMPILE_OPTION "")
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION -e)
endif()

if(CMAKE_C_COMPILER_VERSION_INTERNAL VERSION_GREATER 8)
  set(CMAKE_C11_STANDARD_COMPILE_OPTION "")
  set(CMAKE_C11_EXTENSION_COMPILE_OPTION -e)
endif()

# Architecture specific
if("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM")
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 1.10 90 6.10 99 8.10 11)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "RX")
  __compiler_iar_ilink(C)
  __compiler_check_default_language_standard(C 1.10 90 2.10 99 4.10 11)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "AVR")
  __compiler_iar_xlink(C)
  __compiler_check_default_language_standard(C 7.10 99)
  set(CMAKE_C_OUTPUT_EXTENSION ".r90")

  if(NOT CMAKE_C_LINK_FLAGS)
    set(CMAKE_C_LINK_FLAGS "-Fmotorola")
  endif()

  # add the target specific include directory:
  get_filename_component(_compilerDir "${CMAKE_C_COMPILER}" PATH)
  get_filename_component(_compilerDir "${_compilerDir}" PATH)
  include_directories("${_compilerDir}/inc" )
  include_directories("${_compilerDir}/inc/Atmel" )

else()
  message(FATAL_ERROR "CMAKE_C_COMPILER_ARCHITECTURE_ID not detected. This should be automatic.")
endif()
