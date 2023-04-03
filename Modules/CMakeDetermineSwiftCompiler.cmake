# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)

# Local system-specific compiler preferences for this language.
include(Platform/${CMAKE_SYSTEM_NAME}-Determine-Swift OPTIONAL)
include(Platform/${CMAKE_SYSTEM_NAME}-Swift OPTIONAL)
if(NOT CMAKE_Swift_COMPILER_NAMES)
  set(CMAKE_Swift_COMPILER_NAMES swiftc)
endif()

if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
  if(XCODE_VERSION VERSION_LESS 6.1)
    message(FATAL_ERROR "Swift language not supported by Xcode ${XCODE_VERSION}")
  endif()
  set(CMAKE_Swift_COMPILER_XCODE_TYPE sourcecode.swift)
  execute_process(COMMAND xcrun --find swiftc
    OUTPUT_VARIABLE _xcrun_out OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE _xcrun_err RESULT_VARIABLE _xcrun_result)
  if(_xcrun_result EQUAL 0 AND EXISTS "${_xcrun_out}")
    set(CMAKE_Swift_COMPILER "${_xcrun_out}")
  else()
    _cmake_find_compiler_path(Swift)
  endif()
elseif("${CMAKE_GENERATOR}" MATCHES "^Ninja")
  if(CMAKE_Swift_COMPILER)
    _cmake_find_compiler_path(Swift)
  else()
    set(CMAKE_Swift_COMPILER_INIT NOTFOUND)

    if(NOT $ENV{SWIFTC} STREQUAL "")
      get_filename_component(CMAKE_Swift_COMPILER_INIT $ENV{SWIFTC} PROGRAM
        PROGRAM_ARGS CMAKE_Swift_FLAGS_ENV_INIT)
      if(CMAKE_Swift_FLAGS_ENV_INIT)
        set(CMAKE_Swift_COMPILER_ARG1 "${CMAKE_Swift_FLAGS_ENV_INIT}" CACHE
          STRING "Arguments to the Swift compiler")
      endif()
      if(NOT EXISTS ${CMAKE_Swift_COMPILER_INIT})
        message(FATAL_ERROR "Could not find compiler set in environment variable SWIFTC\n$ENV{SWIFTC}.\n${CMAKE_Swift_COMPILER_INIT}")
      endif()
    endif()

    if(NOT CMAKE_Swift_COMPILER_INIT)
      set(CMAKE_Swift_COMPILER_LIST swiftc ${_CMAKE_TOOLCHAIN_PREFIX}swiftc)
    endif()

    _cmake_find_compiler(Swift)
  endif()
  mark_as_advanced(CMAKE_Swift_COMPILER)
else()
  message(FATAL_ERROR "Swift language not supported by \"${CMAKE_GENERATOR}\" generator")
endif()

# Build a small source file to identify the compiler.
if(NOT CMAKE_Swift_COMPILER_ID_RUN)
  set(CMAKE_Swift_COMPILER_ID_RUN 1)

  if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    list(APPEND CMAKE_Swift_COMPILER_ID_MATCH_VENDORS Apple)
    set(CMAKE_Swift_COMPILER_ID_MATCH_VENDOR_REGEX_Apple "com.apple.xcode.tools.swift.compiler")
  endif()

  # Try to identify the compiler.
  set(CMAKE_Swift_COMPILER_ID)
  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(Swift "" CompilerId/main.swift)
endif()

if (NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_Swift_COMPILER}" PATH)
endif ()

set(_CMAKE_PROCESSING_LANGUAGE "Swift")
include(CMakeFindBinUtils)
unset(_CMAKE_PROCESSING_LANGUAGE)

# configure variables set in this file for fast reload later on
configure_file(${CMAKE_ROOT}/Modules/CMakeSwiftCompiler.cmake.in
               ${CMAKE_PLATFORM_INFO_DIR}/CMakeSwiftCompiler.cmake @ONLY)

set(CMAKE_Swift_COMPILER_ENV_VAR "SWIFTC")
