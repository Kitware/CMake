# This test will verify if CheckSymbolExists only report symbols available
# for linking that really are. You can find some documentation on this in
# bug 11333 where we found out that gcc would optimize out the actual
# reference to the symbol, so symbols that are in fact _not_ available in the
# given libraries (but seen in header) were reported as present.
#
# If you change this test do not forget to change the CheckCXXSymbolExists
# test, too.

PROJECT(CheckSymbolExists C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

SET(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}")

INCLUDE(CheckSymbolExists)

foreach(_config_type Release RelWithDebInfo MinSizeRel Debug)
  set(CMAKE_TRY_COMPILE_CONFIGURATION ${_config_type})
  unset(CSE_RESULT_${_config_type} CACHE)
  MESSAGE(STATUS "Testing configuration ${_config_type}")

  check_symbol_exists(non_existent_function_for_symbol_test "cm_cse.h" CSE_RESULT_${_config_type})

  IF (CSE_RESULT_${_config_type})
    MESSAGE(SEND_ERROR "CheckSymbolExists reported a nonexistent symbol as existing in configuration ${_config_type}")
  ENDIF (CSE_RESULT_${_config_type})
endforeach()

set(CMAKE_TRY_COMPILE_CONFIGURATION ${CMAKE_BUILD_TYPE})
unset(CSE_RESULT_ERRNO CACHE)

check_symbol_exists(errno "errno.h" CSE_RESULT_ERRNO)

IF (NOT CSE_RESULT_ERRNO)
  MESSAGE(SEND_ERROR "CheckSymbolExists did not find errno in <errno.h>")
ELSE (NOT CSE_RESULT_ERRNO)
  MESSAGE(STATUS "errno found as expected")
ENDIF (NOT CSE_RESULT_ERRNO)

IF (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  unset(CSE_RESULT_O3 CACHE)
  MESSAGE(STATUS "Testing with optimization -O3")

  check_symbol_exists(non_existent_function_for_symbol_test "cm_cse.h" CSE_RESULT_O3)

  IF (CSE_RESULT_O3)
    MESSAGE(SEND_ERROR "CheckSymbolExists reported a nonexistent symbol as existing with optimization -O3")
  ENDIF (CSE_RESULT_O3)
ENDIF (CMAKE_COMPILER_IS_GNUCC)
