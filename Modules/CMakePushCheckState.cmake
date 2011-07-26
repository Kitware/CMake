# This module defines two macros:
# CMAKE_PUSH_CHECK_STATE()
# and
# CMAKE_POP_CHECK_STATE()
# These two macros can be used to save and restore the state of the variables
# CMAKE_REQUIRED_FLAGS, CMAKE_REQUIRED_DEFINITIONS, CMAKE_REQUIRED_LIBRARIES
# and CMAKE_REQUIRED_INCLUDES used by the various Check-files coming with CMake,
# like e.g. check_function_exists() etc.
# The variable contents are pushed on a stack, pushing multiple times is supported.
# This is useful e.g. when executing such tests in a Find-module, where they have to be set,
# but after the Find-module has been executed they should have the same value
# as they had before.
#
# Usage:
#   cmake_push_check_state()
#   set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -DSOME_MORE_DEF)
#   check_function_exists(...)
#   cmake_pop_check_state()

#=============================================================================
# Copyright 2006-2011 Alexander Neundorf, <neundorf@kde.org>
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


MACRO(CMAKE_PUSH_CHECK_STATE)

   IF(NOT DEFINED _CMAKE_PUSH_CHECK_STATE_COUNTER)
      SET(_CMAKE_PUSH_CHECK_STATE_COUNTER 0)
   ENDIF()

   MATH(EXPR _CMAKE_PUSH_CHECK_STATE_COUNTER "${_CMAKE_PUSH_CHECK_STATE_COUNTER}+1")

   SET(_CMAKE_REQUIRED_INCLUDES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}    ${CMAKE_REQUIRED_INCLUDES})
   SET(_CMAKE_REQUIRED_DEFINITIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER} ${CMAKE_REQUIRED_DEFINITIONS})
   SET(_CMAKE_REQUIRED_LIBRARIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}   ${CMAKE_REQUIRED_LIBRARIES})
   SET(_CMAKE_REQUIRED_FLAGS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}       ${CMAKE_REQUIRED_FLAGS})
ENDMACRO(CMAKE_PUSH_CHECK_STATE)

MACRO(CMAKE_POP_CHECK_STATE)

# don't pop more than we pushed
   IF("${_CMAKE_PUSH_CHECK_STATE_COUNTER}" GREATER "0")

      SET(CMAKE_REQUIRED_INCLUDES    ${_CMAKE_REQUIRED_INCLUDES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      SET(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      SET(CMAKE_REQUIRED_LIBRARIES   ${_CMAKE_REQUIRED_LIBRARIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      SET(CMAKE_REQUIRED_FLAGS       ${_CMAKE_REQUIRED_FLAGS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})

      MATH(EXPR _CMAKE_PUSH_CHECK_STATE_COUNTER "${_CMAKE_PUSH_CHECK_STATE_COUNTER}-1")
   ENDIF()

ENDMACRO(CMAKE_POP_CHECK_STATE)
