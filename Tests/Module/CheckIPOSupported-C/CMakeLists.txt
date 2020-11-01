cmake_minimum_required(VERSION 3.8)
project(CheckIPOSupported-C LANGUAGES C)

cmake_policy(SET CMP0069 NEW)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMake_TEST_IPO_WORKS_C)
  string(REPLACE "\n" "\n  " ipo_output "${ipo_output}")
  message(FATAL_ERROR "IPO expected to work, but the check failed:\n  ${ipo_output}")
endif()

add_library(foo foo.c)
if(NOT CYGWIN AND (NOT WIN32 OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang"))
  add_library(bar SHARED bar.c)
  if(WIN32)
    # Bindexplib for clang supports LTO objects
    set_target_properties(bar PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif()
else()
  # TODO: bindexplib doesn't support exporting IPO symbols with other compilers on Windows
  add_library(bar STATIC bar.c)
endif()
add_executable(CheckIPOSupported-C main.c)
target_link_libraries(CheckIPOSupported-C PUBLIC foo bar)

enable_testing()
add_test(NAME CheckIPOSupported-C COMMAND CheckIPOSupported-C)
