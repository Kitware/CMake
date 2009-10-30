#
# - Check if the system has the ANSI C files
# CHECK_HEADER_STDC
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
# Copyright (c) 2009, Michihiro NAKAJIMA
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


MACRO (CHECK_HEADER_STDC)
  IF(NOT DEFINED STDC_HEADERS)
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_HEADER_STDC_C_INCLUDE_DIRS "-DINCLUDE_DIRECTORIES=${CMAKE_REQUIRED_INCLUDES}")
    ELSE(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_HEADER_STDC_C_INCLUDE_DIRS)
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    SET(MACRO_CHECK_HEADER_STDC_FLAGS ${CMAKE_REQUIRED_FLAGS})

    MESSAGE(STATUS "Cheking for ANSI C header files")
    TRY_RUN(CHECK_HEADER_STDC_result
      CHECK_HEADER_STDC_compile_result
      ${CMAKE_BINARY_DIR}
      ${libarchive_SOURCE_DIR}/build/cmake/CheckHeaderSTDC.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_HEADER_STDC_FLAGS}
      "${CHECK_HEADER_STDC_C_INCLUDE_DIRS}"
      OUTPUT_VARIABLE OUTPUT)

    IF(CHECK_HEADER_STDC_compile_result AND CHECK_HEADER_STDC_result EQUAL 0)
      FIND_PATH(CHECK_HEADER_STDC_path "string.h")
      IF (CHECK_HEADER_STDC_path)
        FILE(STRINGS "${CHECK_HEADER_STDC_path}/string.h" CHECK_HEADER_STDC_result REGEX "[^a-zA-Z_]memchr[^a-zA-Z_]")
    IF (CHECK_HEADER_STDC_result)
          FILE(STRINGS "${CHECK_HEADER_STDC_path}/stdlib.h" CHECK_HEADER_STDC_result REGEX "[^a-zA-Z_]free[^a-zA-Z_]")
    ENDIF (CHECK_HEADER_STDC_result)
      ENDIF (CHECK_HEADER_STDC_path)
    ENDIF(CHECK_HEADER_STDC_compile_result AND CHECK_HEADER_STDC_result EQUAL 0)

    IF(CHECK_HEADER_STDC_result)
      MESSAGE(STATUS "Cheking for ANSI C header files - found")
      SET(STDC_HEADERS 1 CACHE INTERNAL "Have ANSI C headers")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the include file ${INCLUDE} "
        "exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(CHECK_HEADER_STDC_result)
      MESSAGE(STATUS "Cheking for ANSI C header files - not found")
      SET(STDC_HEADERS "" CACHE INTERNAL "Have ANSI C headers")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the include file ${INCLUDE} "
        "exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(CHECK_HEADER_STDC_result)

  ENDIF(NOT DEFINED STDC_HEADERS)
ENDMACRO (CHECK_HEADER_STDC)

