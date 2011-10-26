# - Find perl
# this module looks for Perl
#
#  PERL_EXECUTABLE - the full path to perl
#  PERL_FOUND      - If false, don't attempt to use perl.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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

include(FindCygwin)

set(PERL_POSSIBLE_BIN_PATHS
  ${CYGWIN_INSTALL_PATH}/bin
  )

if(WIN32)
  get_filename_component(
    ActivePerl_CurrentVersion
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl;CurrentVersion]"
    NAME)
  set(PERL_POSSIBLE_BIN_PATHS ${PERL_POSSIBLE_BIN_PATHS}
    "C:/Perl/bin"
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\${ActivePerl_CurrentVersion}]/bin
    )
endif(WIN32)

find_program(PERL_EXECUTABLE
  NAMES perl
  PATHS ${PERL_POSSIBLE_BIN_PATHS}
  )

# Deprecated settings for compatibility with CMake1.4
set(PERL ${PERL_EXECUTABLE})

# handle the QUIETLY and REQUIRED arguments and set PERL_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Perl DEFAULT_MSG PERL_EXECUTABLE)

mark_as_advanced(PERL_EXECUTABLE)
