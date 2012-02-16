# - CONFIGURE_PACKAGE_CONFIG_FILE(), WRITE_BASIC_PACKAGE_VERSION_FILE()
#
#    CONFIGURE_PACKAGE_CONFIG_FILE(<input> <output> INSTALL_DESTINATION <path>
#                                                   [PATH_VARS <var1> <var2> ... <varN>]
#                                                   [NO_SET_AND_CHECK_MACRO] )
#
# CONFIGURE_PACKAGE_CONFIG_FILE() should be used instead of the plain
# CONFIGURE_FILE() command when creating the <Name>Config.cmake or <Name>-config.cmake
# file for installing a project or library. It helps making the resulting package
# relocatable by avoiding hardcoded paths in the installed Config.cmake file.
#
# In a FooConfig.cmake file there may be code like this to make the
# install destinations know to the using project:
#   set(FOO_INCLUDE_DIR   "@CMAKE_INSTALL_FULL_INCLUDEDIR@" )
#   set(FOO_DATA_DIR   "@CMAKE_INSTALL_PREFIX@/@RELATIVE_DATA_INSTALL_DIR@" )
#   set(FOO_ICONS_DIR   "@CMAKE_INSTALL_PREFIX@/share/icons" )
#   ...logic to determine installedPrefix from the own location...
#   set(FOO_CONFIG_DIR  "${installedPrefix}/@CONFIG_INSTALL_DIR@" )
# All 4 options shown above are not sufficient, since the first 3 hardcode
# the absolute directory locations, and the 4th case works only if the logic
# to determine the installedPrefix is correct, and if CONFIG_INSTALL_DIR contains
# a relative path, which in general cannot be guaranteed.
# This has the effect that the resulting FooConfig.cmake file would work poorly
# under Windows and OSX, where users are used to choose the install location
# of a binary package at install time, independent from how CMAKE_INSTALL_PREFIX
# was set at build/cmake time.
#
# Using CONFIGURE_PACKAGE_CONFIG_FILE() helps. If used correctly, it makes the
# resulting FooConfig.cmake file relocatable.
# Usage:
#   1. write a FooConfig.cmake.in file as you are used to
#   2. insert a line containing only the string "@PACKAGE_INIT@"
#   3. instead of SET(FOO_DIR "@SOME_INSTALL_DIR@"), use SET(FOO_DIR "@PACKAGE_SOME_INSTALL_DIR@")
#      (this must be after the @PACKAGE_INIT@ line)
#   4. instead of using the normal CONFIGURE_FILE(), use CONFIGURE_PACKAGE_CONFIG_FILE()
#
# The <input> and <output> arguments are the input and output file, the same way
# as in CONFIGURE_FILE().
#
# The <path> given to INSTALL_DESTINATION must be the destination where the FooConfig.cmake
# file will be installed to. This can either be a relative or absolute path, both work.
#
# The variables <var1> to <varN> given as PATH_VARS are the variables which contain
# install destinations. For each of them the macro will create a helper variable
# PACKAGE_<var...>. These helper variables must be used
# in the FooConfig.cmake.in file for setting the installed location. They are calculated
# by CONFIGURE_PACKAGE_CONFIG_FILE() so that they are always relative to the
# installed location of the package. This works both for relative and also for absolute locations.
# For absolute locations it works only if the absolute location is a subdirectory
# of CMAKE_INSTALL_PREFIX.
#
# By default configure_package_config_file() also generates a macro set_and_check()
# into the FooConfig.cmake file. This should be used instead of the normal set()
# command for setting directories and file locations. Additionally to setting the
# variable it also checks that the referenced file or directory actually exists
# and fails with a FATAL_ERROR otherwise. This makes sure that the created
# FooConfig.cmake file does not contain wrong references.
# When using the NO_SET_AND_CHECK_MACRO, this macro is not generated into the
# FooConfig.cmake file.
#
# Example:
# CMakeLists.txt:
#   set(INCLUDE_INSTALL_DIR include/ ... CACHE )
#   set(LIB_INSTALL_DIR lib/ ... CACHE )
#   set(SYSCONFIG_INSTALL_DIR etc/foo/ ... CACHE )
#   ...
#   include(CMakePackageConfigHelpers)
#   configure_package_config_file(FooConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FooConfig.cmake
#                                 INSTALL_DESTINATION ${LIB_INSTALL_DIR}/Foo/cmake
#                                 PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FooConfig.cmake DESTINATION ${LIB_INSTALL_DIR}/Foo/cmake )
#
# FooConfig.cmake.in:
#   set(FOO_VERSION x.y.z)
#   ...
#   @PACKAGE_INIT@
#   ...
#   set_and_check(FOO_INCLUDE_DIR "@PACKAGE_INCLUDE_INSTALL_DIR@")
#   set_and_check(FOO_SYSCONFIG_DIR "@PACKAGE_SYSCONFIG_INSTALL_DIR@")


#=============================================================================
# Copyright 2012 Alexander Neundorf <neundorf@kde.org>
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


function(CONFIGURE_PACKAGE_CONFIG_FILE _inputFile _outputFile)
  set(options NO_SET_AND_CHECK_MACRO)
  set(oneValueArgs INSTALL_DESTINATION )
  set(multiValueArgs PATH_VARS )

  cmake_parse_arguments(CCF "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

  if(CCF_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to CONFIGURE_PACKAGE_CONFIG_FILE(): \"${CCF_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT CCF_INSTALL_DESTINATION)
    message(FATAL_ERROR "No INSTALL_DESTINATION given to CONFIGURE_PACKAGE_CONFIG_FILE()")
  endif()

  if(IS_ABSOLUTE "${CCF_INSTALL_DESTINATION}")
    set(absInstallDir "${CCF_INSTALL_DESTINATION}")
  else()
    set(absInstallDir "${CMAKE_INSTALL_PREFIX}/${CCF_INSTALL_DESTINATION}")
  endif()
  file(RELATIVE_PATH PACKAGE_RELATIVE_PATH "${absInstallDir}" "${CMAKE_INSTALL_PREFIX}" )

  foreach(var ${CCF_PATH_VARS})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "Variable ${var} does not exist")
    else()
      if(IS_ABSOLUTE "${${var}}")
        string(REPLACE "${CMAKE_INSTALL_PREFIX}" "\${PACKAGE_PREFIX_DIR}"
                        PACKAGE_${var} "${${var}}")
      else()
        set(PACKAGE_${var} "\${PACKAGE_PREFIX_DIR}/${${var}}")
      endif()
    endif()
  endforeach()

  set(PACKAGE_INIT "
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
get_filename_component(PACKAGE_PREFIX_DIR \"\${CMAKE_CURRENT_LIST_DIR}/${PACKAGE_RELATIVE_PATH}\" ABSOLUTE)
")

  if(NOT CCF_NO_SET_AND_CHECK_MACRO)
    set(PACKAGE_INIT "${PACKAGE_INIT}
macro(set_and_check _var _file)
  set(\${_var} \"\${_file}\")
  if(NOT EXISTS \"\${_file}\")
    message(FATAL_ERROR \"File or directory \${_file} referenced by variable \${_var} does not exist !\")
  endif()
endmacro()
")
  endif()

  set(PACKAGE_INIT "${PACKAGE_INIT}
####################################################################################")

  configure_file("${_inputFile}" "${_outputFile}" @ONLY)

endfunction()
