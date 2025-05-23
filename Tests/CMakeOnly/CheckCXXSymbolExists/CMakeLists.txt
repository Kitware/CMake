# This test will verify if CheckCXXSymbolExists only report symbols available
# for linking that really are. You can find some documentation on this in
# bug 11333 where we found out that gcc would optimize out the actual
# reference to the symbol, so symbols that are in fact _not_ available in the
# given libraries (but seen in header) were reported as present.
#
# If you change this test do not forget to change the CheckSymbolExists
# test, too.

cmake_minimum_required(VERSION 3.10)
project(CheckCXXSymbolExists CXX)

set(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../CheckSymbolExists")

include(CheckCXXSymbolExists)

foreach(_config_type Release RelWithDebInfo MinSizeRel Debug)
  set(CMAKE_TRY_COMPILE_CONFIGURATION ${_config_type})
  unset(CSE_RESULT_${_config_type} CACHE)
  message(STATUS "Testing configuration ${_config_type}")
  check_cxx_symbol_exists(non_existent_function_for_symbol_test "cm_cse.h" CSE_RESULT_${_config_type})

  if (CSE_RESULT_${_config_type})
    message(SEND_ERROR "CheckCXXSymbolExists reported a nonexistent symbol as existing in configuration ${_config_type}")
  endif ()
endforeach()

set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})
unset(CSE_RESULT_ERRNO_CERRNO CACHE)

message(STATUS "Checking <cerrno>")

check_cxx_symbol_exists(errno "cerrno" CSE_RESULT_ERRNO_CERRNO)

if (NOT CSE_RESULT_ERRNO_CERRNO)
  unset(CSE_RESULT_ERRNO_ERRNOH CACHE)

  message(STATUS "Checking <errno.h>")

  check_cxx_symbol_exists(errno "errno.h" CSE_RESULT_ERRNO_ERRNOH)

  if (NOT CSE_RESULT_ERRNO_ERRNOH)
    message(SEND_ERROR "CheckCXXSymbolExists did not find errno in <cerrno> and <errno.h>")
  else ()
    message(STATUS "errno found in <errno.h>")
  endif ()
else ()
  message(STATUS "errno found in <cerrno>")
endif ()

check_cxx_symbol_exists("std::fopen" "cstdio" CSE_RESULT_FOPEN)
if (NOT CSE_RESULT_FOPEN)
  if(NOT ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.10))
    message(SEND_ERROR "CheckCXXSymbolExists did not find std::fopen in <cstdio>")
  endif()
else()
  message(STATUS "std::fopen found in <cstdio>")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|LCC)$")
  string(APPEND CMAKE_CXX_FLAGS " -O3")
  unset(CSE_RESULT_O3 CACHE)
  message(STATUS "Testing with optimization -O3")

  check_cxx_symbol_exists(non_existent_function_for_symbol_test "cm_cse.h" CSE_RESULT_O3)

  if (CSE_RESULT_O3)
    message(SEND_ERROR "CheckCXXSymbolExists reported a nonexistent symbol as existing with optimization -O3")
  endif ()
endif ()

check_cxx_symbol_exists("std::non_existent_function_for_symbol_test<int*>" "algorithm" CSE_RESULT_NON_SYMBOL)
if (CSE_RESULT_NON_SYMBOL)
  message(SEND_ERROR "CheckCXXSymbolExists reported a nonexistent symbol.")
endif()
