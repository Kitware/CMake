# This file is processed when the IAR compiler is used for a C++ file

include(Compiler/IAR)
include(Compiler/CMakeCommonCompilerMacros)

if("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM")
  # "(extended) embedded C++" Mode
  # old version: --ec++ or --eec++
  # since 8.10:  --c++ --no_exceptions --no_rtti
  #
  # --c++ is full C++ and supported since 6.10
  if(NOT CMAKE_IAR_CXX_FLAG)
    if(NOT CMAKE_CXX_COMPILER_VERSION)
      message(FATAL_ERROR "CMAKE_CXX_COMPILER_VERSION not detected.  This should be automatic.")
    endif()
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.10)
      set(CMAKE_IAR_CXX_FLAG --c++)
    else()
      set(CMAKE_IAR_CXX_FLAG --eec++)
    endif()
  endif()

  set(CMAKE_CXX_EXTENSION_COMPILE_OPTION -e)

  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION -e)

  if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.10)
  set(CMAKE_CXX03_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX03_EXTENSION_COMPILE_OPTION -e)
  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION -e)
  set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION -e)
  endif()

  __compiler_iar_ARM(CXX)
  __compiler_check_default_language_standard(CXX 6.10 98 8.10 14)

elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "AVR")
  # "embedded C++" --EC++ is probably closest to CXX98 but with no support for:
  #  Templates, multiple inheritance, virtual inheritance, exceptions, RTTI, C++ style casts,
  #  Namespaces, the mutable attribute, no STL, any library features related to the above features.
  #
  # "(extended) embedded C++" --EEC++ Mode but DOES NOT support any normal C++ standard
  # probably closest to CXX98 but with no RTTI and no exceptions, and the library
  # provided is not in the standard namespace
  if(NOT CMAKE_IAR_CXX_FLAG)
    if(NOT CMAKE_CXX_COMPILER_VERSION)
      message(FATAL_ERROR "CMAKE_CXX_COMPILER_VERSION not detected.  This should be automatic.")
    endif()
    set(CMAKE_IAR_CXX_FLAG --eec++)
  endif()

  set(CMAKE_CXX_EXTENSION_COMPILE_OPTION -e)
  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION -e)

  __compiler_iar_AVR(CXX)
  __compiler_check_default_language_standard(CXX 7.10 98)

  set(CMAKE_CXX_OUTPUT_EXTENSION ".r90")
  if(NOT CMAKE_CXX_LINK_FLAGS)
    set(CMAKE_CXX_LINK_FLAGS "-Fmotorola")
  endif()

  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
  set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -o <TARGET> <OBJECTS> ")

  # add the target specific include directory:
  get_filename_component(_compilerDir "${CMAKE_C_COMPILER}" PATH)
  get_filename_component(_compilerDir "${_compilerDir}" PATH)
  include_directories("${_compilerDir}/inc")
  include_directories("${_compilerDir}/inc/Atmel")

else()
  message(FATAL_ERROR "CMAKE_CXX_COMPILER_ARCHITECTURE_ID not detected as \"AVR\" or \"ARM\".  This should be automatic." )
endif()
