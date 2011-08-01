#  WRITE_BASIC_CONFIG_VERSION_FILE( filename VERSION major.minor.patch COMPATIBILITY (AnyNewerVersion|SameMajorVersion) )
#
# Writes a file for use as <package>ConfigVersion.cmake file to <filename>.
# See the documentation of FIND_PACKAGE() for details on this.
#    filename is the output filename, it should be in the build tree.
#    major.minor.patch is the version number of the project to be installed
# The COMPATIBILITY mode AnyNewerVersion means that the installed package version
# will be considered suitable if it is newer or exactly the same as the requested version.
# If SameMajorVersion is used instead, then the behaviour differs from AnyNewerVersion
# in that the major version number must be the same as requested, e.g. version 2.0 will
# not be considered suitable to 1.0 is requested.
# If you project has more elaborated version matching rules, you will need to write your
# own custom ConfigVersion.cmake file, instead of using this macro.
#
# Internally, this macro executes configure_file() on the input file
# Modules/BasicConfigVersion-AnyNewerVersion/SameMajorVersion.cmake.in to
# create the resulting version file.

# Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

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
