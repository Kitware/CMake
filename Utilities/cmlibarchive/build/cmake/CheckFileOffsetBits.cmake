# - Check if _FILE_OFFSET_BITS macro needed for large files
# CHECK_FILE_OFFSET_BITS ()
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


#include(CheckCXXSourceCompiles)

macro(CHECK_FILE_OFFSET_BITS)

  if(NOT DEFINED _FILE_OFFSET_BITS)
    message(STATUS "Checking _FILE_OFFSET_BITS for large files")
    try_compile(__WITHOUT_FILE_OFFSET_BITS_64
      ${CMAKE_BINARY_DIR}
      ${libarchive_SOURCE_DIR}/build/cmake/CheckFileOffsetBits.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
    if(NOT __WITHOUT_FILE_OFFSET_BITS_64)
      try_compile(__WITH_FILE_OFFSET_BITS_64
        ${CMAKE_BINARY_DIR}
        ${libarchive_SOURCE_DIR}/build/cmake/CheckFileOffsetBits.c
        COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_FILE_OFFSET_BITS=64)
    endif(NOT __WITHOUT_FILE_OFFSET_BITS_64)

    if(NOT __WITHOUT_FILE_OFFSET_BITS_64 AND __WITH_FILE_OFFSET_BITS_64)
      set(_FILE_OFFSET_BITS 64 CACHE INTERNAL "_FILE_OFFSET_BITS macro needed for large files")
      message(STATUS "Checking _FILE_OFFSET_BITS for large files - needed")
    else(NOT __WITHOUT_FILE_OFFSET_BITS_64 AND __WITH_FILE_OFFSET_BITS_64)
      set(_FILE_OFFSET_BITS "" CACHE INTERNAL "_FILE_OFFSET_BITS macro needed for large files")
      message(STATUS "Checking _FILE_OFFSET_BITS for large files - not needed")
    endif(NOT __WITHOUT_FILE_OFFSET_BITS_64 AND __WITH_FILE_OFFSET_BITS_64)
  endif(NOT DEFINED _FILE_OFFSET_BITS)

endmacro(CHECK_FILE_OFFSET_BITS)

