# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindHSPELL
----------

Try to find Hebrew spell-checker (Hspell) and morphology engine.

Once done this will define

::

  HSPELL_FOUND - system has Hspell
  HSPELL_INCLUDE_DIR - the Hspell include directory
  HSPELL_LIBRARIES - The libraries needed to use Hspell
  HSPELL_DEFINITIONS - Compiler switches required for using Hspell



::

  HSPELL_VERSION_STRING - The version of Hspell found (x.y)
  HSPELL_MAJOR_VERSION  - the major version of Hspell
  HSPELL_MINOR_VERSION  - The minor version of Hspell
#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

find_path(HSPELL_INCLUDE_DIR hspell.h)

find_library(HSPELL_LIBRARIES NAMES hspell)

if (HSPELL_INCLUDE_DIR)
    file(STRINGS "${HSPELL_INCLUDE_DIR}/hspell.h" HSPELL_H REGEX "#define HSPELL_VERSION_M(AJO|INO)R [0-9]+")
    string(REGEX REPLACE ".*#define HSPELL_VERSION_MAJOR ([0-9]+).*" "\\1" HSPELL_VERSION_MAJOR "${HSPELL_H}")
    string(REGEX REPLACE ".*#define HSPELL_VERSION_MINOR ([0-9]+).*" "\\1" HSPELL_VERSION_MINOR "${HSPELL_H}")
    set(HSPELL_VERSION_STRING "${HSPELL_VERSION_MAJOR}.${HSPELL_VERSION_MINOR}")
    unset(HSPELL_H)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HSPELL
                                  REQUIRED_VARS HSPELL_LIBRARIES HSPELL_INCLUDE_DIR
                                  VERSION_VAR HSPELL_VERSION_STRING)

mark_as_advanced(HSPELL_INCLUDE_DIR HSPELL_LIBRARIES)

cmake_policy(POP)
