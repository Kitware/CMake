#.rst:
# FindHg
# ------
#
#
#
# The module defines the following variables:
#
# ::
#
#    HG_EXECUTABLE - path to mercurial command line client (hg)
#    HG_FOUND - true if the command line client was found
#    HG_VERSION_STRING - the version of mercurial found
#
# Example usage:
#
# ::
#
#    find_package(Hg)
#    if(HG_FOUND)
#      message("hg found: ${HG_EXECUTABLE}")
#    endif()

#=============================================================================
# Copyright 2010-2012 Kitware, Inc.
# Copyright 2012      Rolf Eike Beer <eike@sf-mail.de>
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

find_program(HG_EXECUTABLE
  NAMES hg
  PATH_SUFFIXES Mercurial
  DOC "hg command line client"
  )
mark_as_advanced(HG_EXECUTABLE)

if(HG_EXECUTABLE)
  execute_process(COMMAND ${HG_EXECUTABLE} --version
                  OUTPUT_VARIABLE hg_version
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(hg_version MATCHES "^Mercurial Distributed SCM \\(version ([0-9][^)]*)\\)")
    set(HG_VERSION_STRING "${CMAKE_MATCH_1}")
  endif()
  unset(hg_version)
endif()

# Handle the QUIETLY and REQUIRED arguments and set HG_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Hg
                                  REQUIRED_VARS HG_EXECUTABLE
                                  VERSION_VAR HG_VERSION_STRING)
