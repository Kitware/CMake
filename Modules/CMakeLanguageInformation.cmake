
#=============================================================================
# Copyright 2015 Kitware, Inc.
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

# This file contains common code blocks used by all the language information
# files

# load any compiler-wrapper specific information
macro(__cmake_include_compiler_wrapper lang)
  set(_INCLUDED_WRAPPER_FILE 0)
  if (CMAKE_${lang}_COMPILER_ID)
    include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_${lang}_COMPILER_WRAPPER}-${CMAKE_${lang}_COMPILER_ID}-${lang} OPTIONAL RESULT_VARIABLE _INCLUDED_WRAPPER_FILE)
  endif()
  if (NOT _INCLUDED_WRAPPER_FILE)
    include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_${lang}_COMPILER_WRAPPER}-${lang} OPTIONAL RESULT_VARIABLE _INCLUDED_WRAPPER_FILE)
  endif ()

  # No platform - wrapper - lang information so maybe there's just wrapper - lang information
  if(NOT _INCLUDED_WRAPPER_FILE)
    if (CMAKE_${lang}_COMPILER_ID)
      include(Compiler/${CMAKE_${lang}_COMPILER_WRAPPER}-${CMAKE_${lang}_COMPILER_ID}-${lang} OPTIONAL RESULT_VARIABLE _INCLUDED_WRAPPER_FILE)
    endif()
    if (NOT _INCLUDED_WRAPPER_FILE)
      include(Compiler/${CMAKE_${lang}_COMPILER_WRAPPER}-${lang} OPTIONAL RESULT_VARIABLE _INCLUDED_WRAPPER_FILE)
    endif ()
  endif ()
endmacro ()
