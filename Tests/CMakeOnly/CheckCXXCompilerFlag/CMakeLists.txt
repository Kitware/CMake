cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
project(CheckCXXCompilerFlag)

message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
set(CMAKE_VERBOSE_MAKEFILE 1)

macro(TEST_FAIL value msg)
  if (${value})
    message (SEND_ERROR "Test fail:" ${msg} ${Out} )
  endif ()
endmacro()

macro(TEST_PASS value msg)
  if (NOT ${value})
    message (SEND_ERROR "Test fail:" ${msg} ${Out} )
  endif ()
endmacro()

if(CMAKE_COMPILER_IS_GNUCXX)
  execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE     _gcc_version_info)
  string (REGEX MATCH "[345]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
  # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the
  # patch level, handle this here:
  if(NOT _gcc_version)
    string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE     _clang_version_info)
  string (REGEX REPLACE ".*version ([0-9]\\.[0-9]).*" "\\1" _clang_version "${_clang_version_info}")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES Intel)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -V OUTPUT_VARIABLE     _intel_version_info)
  string (REGEX REPLACE ".*Version ([0-9]+(\\.[0-9]+)+).*" "\\1" _intel_version "${_intel_version_info}")
endif()

message("Platform:\n  WIN32: ${WIN32}\n  UNIX: ${UNIX}\n  APPLE: ${APPLE}\n  MINGW: ${MINGW}\n  CYGWIN: ${CYGWIN}\n  MSYS: ${MSYS}\n"
  "  MSVC: ${MSVC}\n  MSVC60: ${MSVC60}\n  MSVC70: ${MSVC70}\n  MSVC71: ${MSVC71}\n  MSVC80: ${MSVC80}\n  MSVC90: ${MSVC90}\n  MSVC10: ${MSVC10}\n"
  "  GCC: ${_gcc_version}\n"
  "  Clang: ${_clang_version}\n"
  "  Intel: ${_intel_version}\n"
)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-fvisibility=hidden HAS_HIDDEN_VISIBILITY)

message("HAS_HIDDEN_VISIBILITY: ${HAS_HIDDEN_VISIBILITY}\n\nCOMPILE OUTPUT:\n${OUTPUT}")

if(CMAKE_COMPILER_IS_GNUCXX)
  if(NOT WIN32)
#     test_pass(HAS_HIDDEN_VISIBILITY "GCC should support hidden visibility, but does not.")
  endif()
else()
  message("Unhandled Platform")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC|Clang" AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
  check_cxx_compiler_flag("-x c++" HAVE_X_CXX)
  if(NOT HAVE_X_CXX)
    message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} compiler flag '-x c++' check failed")
  endif()
endif()
