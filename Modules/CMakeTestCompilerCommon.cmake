
#=============================================================================
# Copyright 2010 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

function(PrintTestCompilerStatus LANG MSG)
  IF(CMAKE_GENERATOR MATCHES Make)
    MESSAGE(STATUS "Check for working ${LANG} compiler: ${CMAKE_${LANG}_COMPILER}${MSG}")
  ELSE()
    MESSAGE(STATUS "Check for working ${LANG} compiler using: ${CMAKE_GENERATOR}${MSG}")
  ENDIF()
endfunction()
