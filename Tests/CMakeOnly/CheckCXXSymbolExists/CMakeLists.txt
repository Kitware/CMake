# This test will verify if CheckCXXSymbolExists only report symbols available
# for linking that really are. You can find some documentation on this in
# bug 11333 where we found out that gcc would optimize out the actual
# reference to the symbol, so symbols that are in fact _not_ available in the
# given libraries (but seen in header) were reported as present.
#
# If you change this test do not forget to change the CheckSymbolExists
# test, too.

PROJECT(CheckCXXSymbolExists CXX)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

SET(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../CheckSymbolExists")

INCLUDE(CheckCXXSymbolExists)

foreach(_config_type Release RelWithDebInfo MinSizeRel Debug)
  set(CMAKE_TRY_COMPILE_CONFIGURATION ${_config_type})
  unset(CSE_RESULT_${_config_type} CACHE)
  MESSAGE(STATUS "Testing configuration ${_config_type}")
  check_cxx_symbol_exists(non_existent_function_for_symbol_test "cm_cse.h" CSE_RESULT_${_config_type})

  IF (CSE_RESULT_${_config_type})
    MESSAGE(SEND_ERROR "CheckCXXSymbolExists reported a nonexistent symbol as existing in configuration ${_config_type}")
  ENDIF (CSE_RESULT_${_config_type})
endforeach()

set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})
unset(CSE_RESULT_ERRNO_CERRNO CACHE)

MESSAGE(STATUS "Checking <cerrno>")

check_cxx_symbol_exists(errno "cerrno" CSE_RESULT_ERRNO_CERRNO)

IF (NOT CSE_RESULT_ERRNO_CERRNO)
  unset(CSE_RESULT_ERRNO_ERRNOH CACHE)

  MESSAGE(STATUS "Checking <errno.h>")

  check_cxx_symbol_exists(errno "errno.h" CSE_RESULT_ERRNO_ERRNOH)

  IF (NOT CSE_RESULT_ERRNO_ERRNOH)
    MESSAGE(SEND_ERROR "CheckCXXSymbolExists did not find errno in <cerrno> and <errno.h>")
  ELSE (NOT CSE_RESULT_ERRNO_ERRNOH)
    MESSAGE(STATUS "errno found in <errno.h>")
  ENDIF (NOT CSE_RESULT_ERRNO_ERRNOH)
ELSE (NOT CSE_RESULT_ERRNO_CERRNO)
  MESSAGE(STATUS "errno found in <cerrno>")
ENDIF (NOT CSE_RESULT_ERRNO_CERRNO)

IF (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  unset(CSE_RESULT_O3 CACHE)
  MESSAGE(STATUS "Testing with optimization -O3")

  check_cxx_symbol_exists(non_existent_function_for_symbol_test "cm_cse.h" CSE_RESULT_O3)

  IF (CSE_RESULT_O3)
    MESSAGE(SEND_ERROR "CheckCXXSymbolExists reported a nonexistent symbol as existing with optimization -O3")
  ENDIF (CSE_RESULT_O3)
ENDIF (CMAKE_COMPILER_IS_GNUCXX)
