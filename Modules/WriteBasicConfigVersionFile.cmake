#  WRITE_BASIC_CONFIG_VERSION_FILE( filename VERSION major.minor.patch COMPATIBILITY (AnyNewerVersion|SameMajorVersion) )
#
# Writes a file for use as <package>ConfigVersion.cmake file to <filename>.
# See the documentation of FIND_PACKAGE() for details on this.
#    filename is the output filename, it should be in the build tree.
#    major.minor.patch is the version number of the project to be installed
# The COMPATIBILITY mode AnyNewerVersion means that the installed package version
# will be considered compatible if it is newer or exactly the same as the requested version.
# If SameMajorVersion is used instead, then the behaviour differs from AnyNewerVersion
# in that the major version number must be the same as requested, e.g. version 2.0 will
# not be considered compatible if 1.0 is requested.
# If your project has more elaborated version matching rules, you will need to write your
# own custom ConfigVersion.cmake file instead of using this macro.
#
# Example:
#     write_basic_config_version_file(${CMAKE_CURRENT_BINARY_DIR}/FooConfigVersion.cmake
#                                     VERSION 1.2.3
#                                     COMPATIBILITY SameMajorVersion )
#     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FooConfigVersion.cmake
#                   ${CMAKE_CURRENT_BINARY_DIR}/FooConfig.cmake
#             DESTINATION lib/cmake/Foo )
#
# Internally, this macro executes configure_file() to create the resulting
# version file. Depending on the COMPATIBLITY, either the file
# BasicConfigVersion-SameMajorVersion.cmake.in or BasicConfigVersion-AnyNewerVersion.cmake.in
# is used. Please note that these two files are internal to CMake and you should
# not call configure_file() on them yourself, but they can be used as starting
# point to create more sophisticted custom ConfigVersion.cmake files.

#=============================================================================
# Copyright 2008-2011 Alexander Neundorf, <neundorf@kde.org>
# Copyright 2004-2009 Kitware, Inc.
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

include(CMakeParseArguments)

function(WRITE_BASIC_CONFIG_VERSION_FILE _filename)

  set(options )
  set(oneValueArgs VERSION COMPATIBILITY )
  set(multiValueArgs )

  cmake_parse_arguments(CVF "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

  if(CVF_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to WRITE_BASIC_CONFIG_VERSION_FILE(): \"${CVF_UNPARSED_ARGUMENTS}\"")
  endif(CVF_UNPARSED_ARGUMENTS)

  set(versionTemplateFile "${CMAKE_ROOT}/Modules/BasicConfigVersion-${CVF_COMPATIBILITY}.cmake.in")
  if(NOT EXISTS "${versionTemplateFile}")
    message(FATAL_ERROR "Bad COMPATIBILITY value used for WRITE_BASIC_CONFIG_VERSION_FILE(): \"${CVF_COMPATIBILITY}\"")
  endif()

  if("${CVF_VERSION}" STREQUAL "")
    message(FATAL_ERROR "No VERSION specified for WRITE_BASIC_CONFIG_VERSION_FILE()")
  endif()

  configure_file("${versionTemplateFile}" "${_filename}" @ONLY)

endfunction(WRITE_BASIC_CONFIG_VERSION_FILE)
