# - Test CXX compiler for a flag
# Check if the CXX compiler accepts a flag
#
#  Macro CHECK_CXX_ACCEPTS_FLAG(FLAGS VARIABLE) -
#     checks if the function exists
#  FLAGS - the flags to try
#  VARIABLE - variable to store the result
#

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

macro(CHECK_CXX_ACCEPTS_FLAG FLAGS  VARIABLE)
  if(NOT DEFINED ${VARIABLE})
    message(STATUS "Checking to see if CXX compiler accepts flag ${FLAGS}")
    try_compile(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_ROOT}/Modules/DummyCXXFile.cxx
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${FLAGS}
      OUTPUT_VARIABLE OUTPUT)
    if(${VARIABLE})
      message(STATUS "Checking to see if CXX compiler accepts flag ${FLAGS} - yes")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the CXX compiler accepts the flag ${FLAGS} passed with "
        "the following output:\n${OUTPUT}\n\n")
    else(${VARIABLE})
      message(STATUS "Checking to see if CXX compiler accepts flag ${FLAGS} - no")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the CXX compiler accepts the flag ${FLAGS} failed with "
        "the following output:\n${OUTPUT}\n\n")
    endif(${VARIABLE})
  endif(NOT DEFINED ${VARIABLE})
endmacro(CHECK_CXX_ACCEPTS_FLAG)
