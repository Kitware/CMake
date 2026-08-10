# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


# Do NOT include this module directly into any of your code. It is meant as
# a library for Check*CompilerFlag.cmake and Check*LinkerFlag.cma modules.
# It's content may change in any way between releases.

include_guard(GLOBAL)

macro(CMAKE_CHECK_COMMON_INIT_COMPILE_FLAGS _PREFIX)
  set(${_PREFIX}_EXTRA_CMAKE_ARGUMENTS)
  string(REPLACE "\\;" "\\\\;" CMAKE_REQUIRED_FLAGS
    "${CMAKE_REQUIRED_FLAGS}")
  #[[
  cmake_polify(GET CMP0999 ${_PREFIX}_CMP0999)
  if(${_PREFIX}_CMP0999 STREQUAL "NEW")
    # Join multiple list arguments into the space-separated string that we
    # want. This ensures that the entirety of the value gets passed as
    # compile arguments, which is what the user almost surely intended.
    #
    # FIXME(#27901): This needs a policy to be implemented. This will be
    # available in a future version of CMake.
    list(JOIN CMAKE_REQUIRED_FLAGS " " CMAKE_REQUIRED_FLAGS)
  else()
  #]]
    # If CMAKE_REQUIRED_FLAGS contains an unescaped semicolon, anything after
    # gets passed as flags to 'cmake' itself. This is probably not intended,
    # but we preserve it for compatibility.
    set(${_PREFIX}_EXTRA_CMAKE_ARGUMENTS "${CMAKE_REQUIRED_FLAGS}")
    list(POP_FRONT ${_PREFIX}_EXTRA_CMAKE_ARGUMENTS CMAKE_REQUIRED_FLAGS)
    if(NOT ${_PREFIX}_EXTRA_CMAKE_ARGUMENTS STREQUAL "")
      # cmake_policy(ISSUE_WARNING CMP0999) TODO
      # There are known instances of users accidentally passing '-W' arguments
      # intended for the compiler as arguments to CMake. Since CMake complains
      # about unknown '-W' arguments starting with CMake 4.4, we need to strip
      # these for compatibility.
      string(REPLACE "\\;" "\\\\;" ${_PREFIX}_EXTRA_CMAKE_ARGUMENTS
        "${${_PREFIX}_EXTRA_CMAKE_ARGUMENTS}")
      list(FILTER ${_PREFIX}_EXTRA_CMAKE_ARGUMENTS EXCLUDE REGEX "^-W")
    endif()
  #endif()
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_INCLUDE_DIRECTORIES _PREFIX)
  if(CMAKE_REQUIRED_INCLUDES)
    if(${ARGC} GREATER 1)
      set(${_PREFIX}_INCLUDE_DIRECTORIES
        "-DINCLUDE_DIRECTORIES:STRING=${ARGN};${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(${_PREFIX}_INCLUDE_DIRECTORIES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    endif()
  elseif(${ARGC} GREATER 1)
    set(${_PREFIX}_INCLUDE_DIRECTORIES
      "-DINCLUDE_DIRECTORIES:STRING=${ARGN}")
  else()
    set(${_PREFIX}_INCLUDE_DIRECTORIES)
  endif()
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_LINK_OPTIONS _PREFIX)
  if(CMAKE_REQUIRED_LINK_OPTIONS)
    set(${_PREFIX}_ADD_LINK_OPTIONS
      LINK_OPTIONS ${ARGN} ${CMAKE_REQUIRED_LINK_OPTIONS})
  elseif(${ARGC} GREATER 1)
    set(${_PREFIX}_ADD_LINK_OPTIONS LINK_OPTIONS ${ARGN})
  else()
    set(${_PREFIX}_ADD_LINK_OPTIONS)
  endif()
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_LINK_LIBRARIES _PREFIX)
  if(CMAKE_REQUIRED_LIBRARIES)
    set(${_PREFIX}_ADD_LINK_LIBRARIES
      LINK_LIBRARIES ${ARGN} ${CMAKE_REQUIRED_LIBRARIES})
  elseif(${ARGC} GREATER 1)
    set(${_PREFIX}_ADD_LINK_LIBRARIES LINK_LIBRARIES ${ARGN})
  else()
    set(${_PREFIX}_ADD_LINK_LIBRARIES)
  endif()
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_LINK_DIRECTORIES _PREFIX)
  if(CMAKE_REQUIRED_LINK_DIRECTORIES)
    if(${ARGC} GREATER 1)
      set(${_PREFIX}_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${ARGN};${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(${_PREFIX}_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    endif()
  elseif(${ARGC} GREATER 1)
    set(${_PREFIX}_LINK_DIRECTORIES
      "-DLINK_DIRECTORIES:STRING=${ARGN}")
  else()
    set(${_PREFIX}_LINK_DIRECTORIES)
  endif()
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_COMPILE_ARGS _PREFIX)
  cmake_check_common_init_compile_flags(${_PREFIX})
  cmake_check_common_init_include_directories(${_PREFIX})
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_LINK_ARGS _PREFIX)
  cmake_check_common_init_link_options(${_PREFIX})
  cmake_check_common_init_link_libraries(${_PREFIX})
  cmake_check_common_init_link_directories(${_PREFIX})
endmacro()

macro(CMAKE_CHECK_COMMON_INIT_ARGS _PREFIX)
  cmake_check_common_init_compile_args(${_PREFIX})
  cmake_check_common_init_link_args(${_PREFIX})
endmacro()

macro(CMAKE_CHECK_COMMON_CLEANUP _PREFIX)
  unset(${_PREFIX}_ADD_LINK_OPTIONS)
  unset(${_PREFIX}_ADD_LINK_LIBRARIES)
  unset(${_PREFIX}_LINK_DIRECTORIES)
  unset(${_PREFIX}_INCLUDE_DIRECTORIES)
  unset(${_PREFIX}_EXTRA_CMAKE_ARGUMENTS)
endmacro()
