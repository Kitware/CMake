##section Variables specific to CPack RPM generator
##end
##module
# - The builtin (binary) CPack RPM generator (Unix only)
# CPackRPM may be used to create RPM package using CPack.
# CPackRPM is a CPack generator thus it uses the CPACK_XXX variables
# used by CPack : http://www.cmake.org/Wiki/CMake:CPackConfiguration
#
# However CPackRPM has specific features which are controlled by
# the specifics CPACK_RPM_XXX variables. CPackRPM is a component aware
# generator so when CPACK_RPM_COMPONENT_INSTALL is ON some more
# CPACK_RPM_<ComponentName>_XXXX variables may be used in order
# to have component specific values. Note however that <componentName>
# refers to the **grouping name**. This may be either a component name
# or a component GROUP name.
# Usually those vars correspond to RPM spec file entities, one may find
# information about spec files here http://www.rpm.org/wiki/Docs.
# You'll find a detailed usage of CPackRPM on the wiki:
#  http://www.cmake.org/Wiki/CMake:CPackPackageGenerators#RPM_.28Unix_Only.29
# However as a handy reminder here comes the list of specific variables:
##end
#
##variable
#  CPACK_RPM_PACKAGE_SUMMARY - The RPM package summary.
#     Mandatory : YES
#     Default   : CPACK_PACKAGE_DESCRIPTION_SUMMARY
##end
##variable
#  CPACK_RPM_PACKAGE_NAME - The RPM package name.
#     Mandatory : YES
#     Default   : CPACK_PACKAGE_NAME
##end
##variable
#  CPACK_RPM_PACKAGE_VERSION - The RPM package version.
#     Mandatory : YES
#     Default   : CPACK_PACKAGE_VERSION
##end
##variable
#  CPACK_RPM_PACKAGE_ARCHITECTURE - The RPM package architecture.
#     Mandatory : NO
#     Default   : -
#     This may be set to "noarch" if you
#     know you are building a noarch package.
##end
##variable
#  CPACK_RPM_PACKAGE_RELEASE - The RPM package release.
#     Mandatory : YES
#     Default   : 1
#     This is the numbering of the RPM package
#     itself, i.e. the version of the packaging and not the version of the
#     content (see CPACK_RPM_PACKAGE_VERSION). One may change the default
#     value if the previous packaging was buggy and/or you want to put here
#     a fancy Linux distro specific numbering.
##end
##variable
#  CPACK_RPM_PACKAGE_LICENSE - The RPM package license policy.
#     Mandatory : YES
#     Default   : "unknown"
##end
##variable
#  CPACK_RPM_PACKAGE_GROUP - The RPM package group.
#     Mandatory : YES
#     Default   : "unknown"
##end
##variable
#  CPACK_RPM_PACKAGE_VENDOR - The RPM package vendor.
#     Mandatory : YES
#     Default   : CPACK_PACKAGE_VENDOR if set or "unknown"
##end
##variable
#  CPACK_RPM_PACKAGE_URL - The projects URL.
#     Mandatory : NO
#     Default   : -
##end
##variable
#  CPACK_RPM_PACKAGE_DESCRIPTION - RPM package description.
#     Mandatory : YES
#     Default   : CPACK_PACKAGE_DESCRIPTION_FILE if set or "no package description available"
##end
##variable
#  CPACK_RPM_COMPRESSION_TYPE - RPM compression type.
#     Mandatory : NO
#     Default   : -
#     May be used to override RPM compression type to be used
#     to build the RPM. For example some Linux distribution now default
#     to lzma or xz compression whereas older cannot use such RPM.
#     Using this one can enforce compression type to be used.
#     Possible value are: lzma, xz, bzip2 and gzip.
##end
##variable
#  CPACK_RPM_PACKAGE_REQUIRES - RPM spec requires field.
#     Mandatory : NO
#     Default   : -
#     May be used to set RPM dependencies (requires).
#     Note that you must enclose the complete requires string between quotes,
#     for example:
#     set(CPACK_RPM_PACKAGE_REQUIRES "python >= 2.5.0, cmake >= 2.8")
#     The required package list of an RPM file could be printed with
#     rpm -qp --requires file.rpm
##end
##variable
#  CPACK_RPM_PACKAGE_SUGGESTS - RPM spec suggest field.
#     Mandatory : NO
#     Default   : -
#     May be used to set weak RPM dependencies (suggests).
#     Note that you must enclose the complete requires string between quotes.
##end
##variable
#  CPACK_RPM_PACKAGE_PROVIDES - RPM spec provides field.
#     Mandatory : NO
#     Default   : -
#     May be used to set RPM dependencies (provides).
#     The provided package list of an RPM file could be printed with
#     rpm -qp --provides file.rpm
##end
##variable
#  CPACK_RPM_PACKAGE_OBSOLETES - RPM spec obsoletes field.
#     Mandatory : NO
#     Default   : -
#     May be used to set RPM packages that are obsoleted by this one.
##end
##variable
#  CPACK_RPM_PACKAGE_RELOCATABLE - build a relocatable RPM.
#     Mandatory : NO
#     Default   : CPACK_PACKAGE_RELOCATABLE
#     If this variable is set to TRUE or ON CPackRPM will try
#     to build a relocatable RPM package. A relocatable RPM may
#     be installed using rpm --prefix or --relocate in order to
#     install it at an alternate place see rpm(8).
#     Note that currently this may fail if CPACK_SET_DESTDIR is set to ON.
#     If CPACK_SET_DESTDIR is set then you will get a warning message
#     but if there is file installed with absolute path you'll get
#     unexpected behavior.
##end
##variable
#  CPACK_RPM_SPEC_INSTALL_POST - [deprecated].
#     Mandatory : NO
#     Default   : -
#     This way of specifying post-install script is deprecated use
#     CPACK_RPM_POST_INSTALL_SCRIPT_FILE
#     May be used to set an RPM post-install command inside the spec file.
#     For example setting it to "/bin/true" may be used to prevent
#     rpmbuild to strip binaries.
##end
##variable
#  CPACK_RPM_SPEC_MORE_DEFINE - RPM extended spec definitions lines.
#     Mandatory : NO
#     Default   : -
#     May be used to add any %define lines to the generated spec file.
##end
##variable
#  CPACK_RPM_PACKAGE_DEBUG - Toggle CPackRPM debug output.
#     Mandatory : NO
#     Default   : -
#     May be set when invoking cpack in order to trace debug information
#     during CPack RPM run. For example you may launch CPack like this
#     cpack -D CPACK_RPM_PACKAGE_DEBUG=1 -G RPM
##end
##variable
#  CPACK_RPM_USER_BINARY_SPECFILE - A user provided spec file.
#     Mandatory : NO
#     Default   : -
#     May be set by the user in order to specify a USER binary spec file
#     to be used by CPackRPM instead of generating the file.
#     The specified file will be processed by configure_file( @ONLY).
##end
##variable
#  CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE - Spec file template.
#     Mandatory : NO
#     Default   : -
#     If set CPack will generate a template for USER specified binary
#     spec file and stop with an error. For example launch CPack like this
#     cpack -D CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE=1 -G RPM
#     The user may then use this file in order to hand-craft is own
#     binary spec file which may be used with CPACK_RPM_USER_BINARY_SPECFILE.
##end
##variable
#  CPACK_RPM_PRE_INSTALL_SCRIPT_FILE
#  CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
#     Mandatory : NO
#     Default   : -
#     May be used to embed a pre (un)installation script in the spec file.
#     The refered script file(s) will be read and directly
#     put after the %pre or %preun section
#     If CPACK_RPM_COMPONENT_INSTALL is set to ON the (un)install script for
#     each component can be overridden with
#     CPACK_RPM_<COMPONENT>_PRE_INSTALL_SCRIPT_FILE and
#     CPACK_RPM_<COMPONENT>_PRE_UNINSTALL_SCRIPT_FILE
#     One may verify which scriptlet has been included with
#      rpm -qp --scripts  package.rpm
##end
##variable
#  CPACK_RPM_POST_INSTALL_SCRIPT_FILE
#  CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
#     Mandatory : NO
#     Default   : -
#     May be used to embed a post (un)installation script in the spec file.
#     The refered script file(s) will be read and directly
#     put after the %post or %postun section
#     If CPACK_RPM_COMPONENT_INSTALL is set to ON the (un)install script for
#     each component can be overridden with
#     CPACK_RPM_<COMPONENT>_POST_INSTALL_SCRIPT_FILE and
#     CPACK_RPM_<COMPONENT>_POST_UNINSTALL_SCRIPT_FILE
#     One may verify which scriptlet has been included with
#      rpm -qp --scripts  package.rpm
##end
##variable
#  CPACK_RPM_USER_FILELIST
#  CPACK_RPM_<COMPONENT>_USER_FILELIST
#     Mandatory : NO
#     Default   : -
#     May be used to explicitly specify %(<directive>) file line
#     in the spec file. Like %config(noreplace) or any other directive
#     that be found in the %files section. Since CPackRPM is generating
#     the list of files (and directories) the user specified files of
#     the CPACK_RPM_<COMPONENT>_USER_FILELIST list will be removed from the generated list.
##end
##variable
#  CPACK_RPM_CHANGELOG_FILE - RPM changelog file.
#     Mandatory : NO
#     Default   : -
#     May be used to embed a changelog in the spec file.
#     The refered file will be read and directly put after the %changelog
#     section.
##end

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

# Author: Eric Noulard with the help of Alexander Neundorf.

if(CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CPackRPM.cmake may only be used by CPack internally.")
endif(CMAKE_BINARY_DIR)

if(NOT UNIX)
  message(FATAL_ERROR "CPackRPM.cmake may only be used under UNIX.")
endif(NOT UNIX)

# rpmbuild is the basic command for building RPM package
# it may be a simple (symbolic) link to rpm command.
find_program(RPMBUILD_EXECUTABLE rpmbuild)

# Check version of the rpmbuild tool this would be easier to
# track bugs with users and CPackRPM debug mode.
# We may use RPM version in order to check for available version dependent features
if(RPMBUILD_EXECUTABLE)
  execute_process(COMMAND ${RPMBUILD_EXECUTABLE} --version
                  OUTPUT_VARIABLE _TMP_VERSION
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^.*\ " ""
         RPMBUILD_EXECUTABLE_VERSION
         ${_TMP_VERSION})
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: rpmbuild version is <${RPMBUILD_EXECUTABLE_VERSION}>")
  endif(CPACK_RPM_PACKAGE_DEBUG)
endif(RPMBUILD_EXECUTABLE)

if(NOT RPMBUILD_EXECUTABLE)
  message(FATAL_ERROR "RPM package requires rpmbuild executable")
endif(NOT RPMBUILD_EXECUTABLE)

# Display lsb_release output if DEBUG mode enable
# This will help to diagnose problem with CPackRPM
# because we will know on which kind of Linux we are
if(CPACK_RPM_PACKAGE_DEBUG)
  find_program(LSB_RELEASE_EXECUTABLE lsb_release)
  if(LSB_RELEASE_EXECUTABLE)
    execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -a
                    OUTPUT_VARIABLE _TMP_LSB_RELEASE_OUTPUT
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "\n" ", "
           LSB_RELEASE_OUTPUT
           ${_TMP_LSB_RELEASE_OUTPUT})
  else (LSB_RELEASE_EXECUTABLE)
    set(LSB_RELEASE_OUTPUT "lsb_release not installed/found!")
  endif(LSB_RELEASE_EXECUTABLE)
  message("CPackRPM:Debug: LSB_RELEASE  = ${LSB_RELEASE_OUTPUT}")
endif(CPACK_RPM_PACKAGE_DEBUG)

# We may use RPM version in the future in order
# to shut down warning about space in buildtree
# some recent RPM version should support space in different places.
# not checked [yet].
if(CPACK_TOPLEVEL_DIRECTORY MATCHES ".* .*")
  message(FATAL_ERROR "${RPMBUILD_EXECUTABLE} can't handle paths with spaces, use a build directory without spaces for building RPMs.")
endif(CPACK_TOPLEVEL_DIRECTORY MATCHES ".* .*")

# If rpmbuild is found
# we try to discover alien since we may be on non RPM distro like Debian.
# In this case we may try to to use more advanced features
# like generating RPM directly from DEB using alien.
# FIXME feature not finished (yet)
find_program(ALIEN_EXECUTABLE alien)
if(ALIEN_EXECUTABLE)
  message(STATUS "alien found, we may be on a Debian based distro.")
endif(ALIEN_EXECUTABLE)

# Are we packaging components ?
if(CPACK_RPM_PACKAGE_COMPONENT)
  set(CPACK_RPM_PACKAGE_COMPONENT_PART_NAME "-${CPACK_RPM_PACKAGE_COMPONENT}")
  set(CPACK_RPM_PACKAGE_COMPONENT_PART_PATH "/${CPACK_RPM_PACKAGE_COMPONENT}")
  set(WDIR "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/${CPACK_RPM_PACKAGE_COMPONENT}")
else(CPACK_RPM_PACKAGE_COMPONENT)
  set(CPACK_RPM_PACKAGE_COMPONENT_PART_NAME "")
  set(CPACK_RPM_PACKAGE_COMPONENT_PART_PATH "")
  set(WDIR "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}")
endif(CPACK_RPM_PACKAGE_COMPONENT)

#
# Use user-defined RPM specific variables value
# or generate reasonable default value from
# CPACK_xxx generic values.
# The variables comes from the needed (mandatory or not)
# values found in the RPM specification file aka ".spec" file.
# The variables which may/should be defined are:
#

# CPACK_RPM_PACKAGE_SUMMARY (mandatory)
if(NOT CPACK_RPM_PACKAGE_SUMMARY)
  # if neither var is defined lets use the name as summary
  if(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_RPM_PACKAGE_SUMMARY)
  else(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    set(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
  endif(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
endif(NOT CPACK_RPM_PACKAGE_SUMMARY)

# CPACK_RPM_PACKAGE_NAME (mandatory)
if(NOT CPACK_RPM_PACKAGE_NAME)
  string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_RPM_PACKAGE_NAME)
endif(NOT CPACK_RPM_PACKAGE_NAME)

# CPACK_RPM_PACKAGE_VERSION (mandatory)
if(NOT CPACK_RPM_PACKAGE_VERSION)
  if(NOT CPACK_PACKAGE_VERSION)
    message(FATAL_ERROR "RPM package requires a package version")
  endif(NOT CPACK_PACKAGE_VERSION)
  set(CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
endif(NOT CPACK_RPM_PACKAGE_VERSION)
# Replace '-' in version with '_'
# '-' character is  an Illegal RPM version character
# it is illegal because it is used to separate
# RPM "Version" from RPM "Release"
string(REPLACE "-" "_" CPACK_RPM_PACKAGE_VERSION ${CPACK_RPM_PACKAGE_VERSION})

# CPACK_RPM_PACKAGE_ARCHITECTURE (optional)
if(CPACK_RPM_PACKAGE_ARCHITECTURE)
  set(TMP_RPM_BUILDARCH "Buildarch: ${CPACK_RPM_PACKAGE_ARCHITECTURE}")
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: using user-specified build arch = ${CPACK_RPM_PACKAGE_ARCHITECTURE}")
  endif(CPACK_RPM_PACKAGE_DEBUG)
else(CPACK_RPM_PACKAGE_ARCHITECTURE)
  set(TMP_RPM_BUILDARCH "")
endif(CPACK_RPM_PACKAGE_ARCHITECTURE)

# CPACK_RPM_PACKAGE_RELEASE
# The RPM release is the numbering of the RPM package ITSELF
# this is the version of the PACKAGING and NOT the version
# of the CONTENT of the package.
# You may well need to generate a new RPM package release
# without changing the version of the packaged software.
# This is the case when the packaging is buggy (not) the software :=)
# If not set, 1 is a good candidate
if(NOT CPACK_RPM_PACKAGE_RELEASE)
  set(CPACK_RPM_PACKAGE_RELEASE 1)
endif(NOT CPACK_RPM_PACKAGE_RELEASE)

# CPACK_RPM_PACKAGE_LICENSE
if(NOT CPACK_RPM_PACKAGE_LICENSE)
  set(CPACK_RPM_PACKAGE_LICENSE "unknown")
endif(NOT CPACK_RPM_PACKAGE_LICENSE)

# CPACK_RPM_PACKAGE_GROUP
if(NOT CPACK_RPM_PACKAGE_GROUP)
  set(CPACK_RPM_PACKAGE_GROUP "unknown")
endif(NOT CPACK_RPM_PACKAGE_GROUP)

# CPACK_RPM_PACKAGE_VENDOR
if(NOT CPACK_RPM_PACKAGE_VENDOR)
  if(CPACK_PACKAGE_VENDOR)
    set(CPACK_RPM_PACKAGE_VENDOR "${CPACK_PACKAGE_VENDOR}")
  else(CPACK_PACKAGE_VENDOR)
    set(CPACK_RPM_PACKAGE_VENDOR "unknown")
  endif(CPACK_PACKAGE_VENDOR)
endif(NOT CPACK_RPM_PACKAGE_VENDOR)

# CPACK_RPM_PACKAGE_SOURCE
# The name of the source tarball in case we generate a source RPM

# CPACK_RPM_PACKAGE_DESCRIPTION
# The variable content may be either
#   - explicitly given by the user or
#   - filled with the content of CPACK_PACKAGE_DESCRIPTION_FILE
#     if it is defined
#   - set to a default value
#
if(NOT CPACK_RPM_PACKAGE_DESCRIPTION)
        if(CPACK_PACKAGE_DESCRIPTION_FILE)
                file(READ ${CPACK_PACKAGE_DESCRIPTION_FILE} CPACK_RPM_PACKAGE_DESCRIPTION)
        else(CPACK_PACKAGE_DESCRIPTION_FILE)
                set(CPACK_RPM_PACKAGE_DESCRIPTION "no package description available")
        endif(CPACK_PACKAGE_DESCRIPTION_FILE)
endif(NOT CPACK_RPM_PACKAGE_DESCRIPTION)

# CPACK_RPM_COMPRESSION_TYPE
#
if(CPACK_RPM_COMPRESSION_TYPE)
   if(CPACK_RPM_PACKAGE_DEBUG)
     message("CPackRPM:Debug: User Specified RPM compression type: ${CPACK_RPM_COMPRESSION_TYPE}")
   endif(CPACK_RPM_PACKAGE_DEBUG)
   if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "lzma")
     set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w9.lzdio")
   endif(CPACK_RPM_COMPRESSION_TYPE STREQUAL "lzma")
   if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "xz")
     set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w7.xzdio")
   endif(CPACK_RPM_COMPRESSION_TYPE STREQUAL "xz")
   if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "bzip2")
     set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w9.bzdio")
   endif(CPACK_RPM_COMPRESSION_TYPE STREQUAL "bzip2")
   if(CPACK_RPM_COMPRESSION_TYPE STREQUAL "gzip")
     set(CPACK_RPM_COMPRESSION_TYPE_TMP "%define _binary_payload w9.gzdio")
   endif(CPACK_RPM_COMPRESSION_TYPE STREQUAL "gzip")
else(CPACK_RPM_COMPRESSION_TYPE)
   set(CPACK_RPM_COMPRESSION_TYPE_TMP "")
endif(CPACK_RPM_COMPRESSION_TYPE)

if(CPACK_PACKAGE_RELOCATABLE)
  set(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)
endif(CPACK_PACKAGE_RELOCATABLE)
if(CPACK_RPM_PACKAGE_RELOCATABLE)
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: Trying to build a relocatable package")
  endif(CPACK_RPM_PACKAGE_DEBUG)
  if(CPACK_SET_DESTDIR AND (NOT CPACK_SET_DESTDIR STREQUAL "I_ON"))
    message("CPackRPM:Warning: CPACK_SET_DESTDIR is set (=${CPACK_SET_DESTDIR}) while requesting a relocatable package (CPACK_RPM_PACKAGE_RELOCATABLE is set): this is not supported, the package won't be relocatable.")
  else(CPACK_SET_DESTDIR AND (NOT CPACK_SET_DESTDIR STREQUAL "I_ON"))
    set(CPACK_RPM_PACKAGE_PREFIX ${CPACK_PACKAGING_INSTALL_PREFIX})
  endif(CPACK_SET_DESTDIR AND (NOT CPACK_SET_DESTDIR STREQUAL "I_ON"))
endif(CPACK_RPM_PACKAGE_RELOCATABLE)

# Check if additional fields for RPM spec header are given
# There may be some COMPONENT specific variables as well
foreach(_RPM_SPEC_HEADER URL REQUIRES SUGGESTS PROVIDES OBSOLETES PREFIX CONFLICTS AUTOPROV AUTOREQ AUTOREQPROV)
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: processing ${_RPM_SPEC_HEADER}")
    endif(CPACK_RPM_PACKAGE_DEBUG)
    if(CPACK_RPM_PACKAGE_COMPONENT)
        if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER})
            if(CPACK_RPM_PACKAGE_DEBUG)
              message("CPackRPM:Debug: using CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER}")
            endif(CPACK_RPM_PACKAGE_DEBUG)
            set(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER}})
        else()
            if(CPACK_RPM_PACKAGE_DEBUG)
              message("CPackRPM:Debug: CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PACKAGE_${_RPM_SPEC_HEADER} not defined")
              message("CPackRPM:Debug: using CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}")
            endif(CPACK_RPM_PACKAGE_DEBUG)
            set(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}})
        endif()
    else()
        if(CPACK_RPM_PACKAGE_DEBUG)
          message("CPackRPM:Debug: using CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}")
        endif(CPACK_RPM_PACKAGE_DEBUG)
        set(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}})
    endif()

  if(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP)
    string(LENGTH ${_RPM_SPEC_HEADER} _PACKAGE_HEADER_STRLENGTH)
    math(EXPR _PACKAGE_HEADER_STRLENGTH "${_PACKAGE_HEADER_STRLENGTH} - 1")
    string(SUBSTRING ${_RPM_SPEC_HEADER} 1 ${_PACKAGE_HEADER_STRLENGTH} _PACKAGE_HEADER_TAIL)
    string(TOLOWER "${_PACKAGE_HEADER_TAIL}" _PACKAGE_HEADER_TAIL)
    string(SUBSTRING ${_RPM_SPEC_HEADER} 0 1 _PACKAGE_HEADER_NAME)
    set(_PACKAGE_HEADER_NAME "${_PACKAGE_HEADER_NAME}${_PACKAGE_HEADER_TAIL}")
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: User defined ${_PACKAGE_HEADER_NAME}:\n ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP}")
    endif(CPACK_RPM_PACKAGE_DEBUG)
    set(TMP_RPM_${_RPM_SPEC_HEADER} "${_PACKAGE_HEADER_NAME}: ${CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP}")
  else(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP)
    # Do not forget to unset previously set header (from previous component)
    unset(TMP_RPM_${_RPM_SPEC_HEADER})
  endif(CPACK_RPM_PACKAGE_${_RPM_SPEC_HEADER}_TMP)
endforeach(_RPM_SPEC_HEADER)

# CPACK_RPM_SPEC_INSTALL_POST
# May be used to define a RPM post intallation script
# for example setting it to "/bin/true" may prevent
# rpmbuild from stripping binaries.
if(CPACK_RPM_SPEC_INSTALL_POST)
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: User defined CPACK_RPM_SPEC_INSTALL_POST = ${CPACK_RPM_SPEC_INSTALL_POST}")
  endif(CPACK_RPM_PACKAGE_DEBUG)
  set(TMP_RPM_SPEC_INSTALL_POST "%define __spec_install_post ${CPACK_RPM_SPEC_INSTALL_POST}")
endif(CPACK_RPM_SPEC_INSTALL_POST)

# CPACK_RPM_POST_INSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_POST_INSTALL_SCRIPT_FILE)
# CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_POST_UNINSTALL_SCRIPT_FILE)
# May be used to embed a post (un)installation script in the spec file.
# The refered script file(s) will be read and directly
# put after the %post or %postun section
if(CPACK_RPM_PACKAGE_COMPONENT)
  if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_INSTALL_SCRIPT_FILE)
    set(CPACK_RPM_POST_INSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_INSTALL_SCRIPT_FILE})
  else()
    set(CPACK_RPM_POST_INSTALL_READ_FILE ${CPACK_RPM_POST_INSTALL_SCRIPT_FILE})
  endif()
  if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_UNINSTALL_SCRIPT_FILE)
    set(CPACK_RPM_POST_UNINSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_UNINSTALL_SCRIPT_FILE})
  else(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_UNINSTALL_SCRIPT_FILE)
    set(CPACK_RPM_POST_UNINSTALL_READ_FILE ${CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE})
  endif(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_POST_UNINSTALL_SCRIPT_FILE)
else(CPACK_RPM_PACKAGE_COMPONENT)
  set(CPACK_RPM_POST_INSTALL_READ_FILE ${CPACK_RPM_POST_INSTALL_SCRIPT_FILE})
  set(CPACK_RPM_POST_UNINSTALL_READ_FILE ${CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE})
endif(CPACK_RPM_PACKAGE_COMPONENT)

# Handle post-install file if it has been specified
if(CPACK_RPM_POST_INSTALL_READ_FILE)
  if(EXISTS ${CPACK_RPM_POST_INSTALL_READ_FILE})
    file(READ ${CPACK_RPM_POST_INSTALL_READ_FILE} CPACK_RPM_SPEC_POSTINSTALL)
  else(EXISTS ${CPACK_RPM_POST_INSTALL_READ_FILE})
    message("CPackRPM:Warning: CPACK_RPM_POST_INSTALL_SCRIPT_FILE <${CPACK_RPM_POST_INSTALL_READ_FILE}> does not exists - ignoring")
  endif(EXISTS ${CPACK_RPM_POST_INSTALL_READ_FILE})
else(CPACK_RPM_POST_INSTALL_READ_FILE)
  # reset SPEC var value if no post install file has been specified
  # (either globally or component-wise)
  set(CPACK_RPM_SPEC_POSTINSTALL "")
endif(CPACK_RPM_POST_INSTALL_READ_FILE)

# Handle post-uninstall file if it has been specified
if(CPACK_RPM_POST_UNINSTALL_READ_FILE)
  if(EXISTS ${CPACK_RPM_POST_UNINSTALL_READ_FILE})
    file(READ ${CPACK_RPM_POST_UNINSTALL_READ_FILE} CPACK_RPM_SPEC_POSTUNINSTALL)
  else(EXISTS ${CPACK_RPM_POST_UNINSTALL_READ_FILE})
    message("CPackRPM:Warning: CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE <${CPACK_RPM_POST_UNINSTALL_READ_FILE}> does not exists - ignoring")
  endif(EXISTS ${CPACK_RPM_POST_UNINSTALL_READ_FILE})
else(CPACK_RPM_POST_UNINSTALL_READ_FILE)
  # reset SPEC var value if no post uninstall file has been specified
  # (either globally or component-wise)
  set(CPACK_RPM_SPEC_POSTUNINSTALL "")
endif(CPACK_RPM_POST_UNINSTALL_READ_FILE)

# CPACK_RPM_PRE_INSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_PRE_INSTALL_SCRIPT_FILE)
# CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE (or CPACK_RPM_<COMPONENT>_PRE_UNINSTALL_SCRIPT_FILE)
# May be used to embed a pre (un)installation script in the spec file.
# The refered script file(s) will be read and directly
# put after the %pre or %preun section
if(CPACK_RPM_PACKAGE_COMPONENT)
  if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_INSTALL_SCRIPT_FILE)
    set(CPACK_RPM_PRE_INSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_INSTALL_SCRIPT_FILE})
  else(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_INSTALL_SCRIPT_FILE)
    set(CPACK_RPM_PRE_INSTALL_READ_FILE ${CPACK_RPM_PRE_INSTALL_SCRIPT_FILE})
  endif(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_INSTALL_SCRIPT_FILE)
  if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_UNINSTALL_SCRIPT_FILE)
    set(CPACK_RPM_PRE_UNINSTALL_READ_FILE ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_UNINSTALL_SCRIPT_FILE})
  else(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_UNINSTALL_SCRIPT_FILE)
    set(CPACK_RPM_PRE_UNINSTALL_READ_FILE ${CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE})
  endif(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_PRE_UNINSTALL_SCRIPT_FILE)
else(CPACK_RPM_PACKAGE_COMPONENT)
  set(CPACK_RPM_PRE_INSTALL_READ_FILE ${CPACK_RPM_PRE_INSTALL_SCRIPT_FILE})
  set(CPACK_RPM_PRE_UNINSTALL_READ_FILE ${CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE})
endif(CPACK_RPM_PACKAGE_COMPONENT)

# Handle pre-install file if it has been specified
if(CPACK_RPM_PRE_INSTALL_READ_FILE)
  if(EXISTS ${CPACK_RPM_PRE_INSTALL_READ_FILE})
    file(READ ${CPACK_RPM_PRE_INSTALL_READ_FILE} CPACK_RPM_SPEC_PREINSTALL)
  else(EXISTS ${CPACK_RPM_PRE_INSTALL_READ_FILE})
    message("CPackRPM:Warning: CPACK_RPM_PRE_INSTALL_SCRIPT_FILE <${CPACK_RPM_PRE_INSTALL_READ_FILE}> does not exists - ignoring")
  endif(EXISTS ${CPACK_RPM_PRE_INSTALL_READ_FILE})
else(CPACK_RPM_PRE_INSTALL_READ_FILE)
  # reset SPEC var value if no pre-install file has been specified
  # (either globally or component-wise)
  set(CPACK_RPM_SPEC_PREINSTALL "")
endif(CPACK_RPM_PRE_INSTALL_READ_FILE)

# Handle pre-uninstall file if it has been specified
if(CPACK_RPM_PRE_UNINSTALL_READ_FILE)
  if(EXISTS ${CPACK_RPM_PRE_UNINSTALL_READ_FILE})
    file(READ ${CPACK_RPM_PRE_UNINSTALL_READ_FILE} CPACK_RPM_SPEC_PREUNINSTALL)
  else(EXISTS ${CPACK_RPM_PRE_UNINSTALL_READ_FILE})
    message("CPackRPM:Warning: CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE <${CPACK_RPM_PRE_UNINSTALL_READ_FILE}> does not exists - ignoring")
  endif(EXISTS ${CPACK_RPM_PRE_UNINSTALL_READ_FILE})
else(CPACK_RPM_PRE_UNINSTALL_READ_FILE)
  # reset SPEC var value if no pre-uninstall file has been specified
  # (either globally or component-wise)
  set(CPACK_RPM_SPEC_PREUNINSTALL "")
endif(CPACK_RPM_PRE_UNINSTALL_READ_FILE)

# CPACK_RPM_CHANGELOG_FILE
# May be used to embed a changelog in the spec file.
# The refered file will be read and directly put after the %changelog section
if(CPACK_RPM_CHANGELOG_FILE)
  if(EXISTS ${CPACK_RPM_CHANGELOG_FILE})
    file(READ ${CPACK_RPM_CHANGELOG_FILE} CPACK_RPM_SPEC_CHANGELOG)
  else(EXISTS ${CPACK_RPM_CHANGELOG_FILE})
    message(SEND_ERROR "CPackRPM:Warning: CPACK_RPM_CHANGELOG_FILE <${CPACK_RPM_CHANGELOG_FILE}> does not exists - ignoring")
  endif(EXISTS ${CPACK_RPM_CHANGELOG_FILE})
else(CPACK_RPM_CHANGELOG_FILE)
  set(CPACK_RPM_SPEC_CHANGELOG "* Sun Jul 4 2010 Erk <eric.noulard@gmail.com>\n  Generated by CPack RPM (no Changelog file were provided)")
endif(CPACK_RPM_CHANGELOG_FILE)

# CPACK_RPM_SPEC_MORE_DEFINE
# This is a generated spec rpm file spaceholder
if(CPACK_RPM_SPEC_MORE_DEFINE)
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: User defined more define spec line specified:\n ${CPACK_RPM_SPEC_MORE_DEFINE}")
  endif(CPACK_RPM_PACKAGE_DEBUG)
endif(CPACK_RPM_SPEC_MORE_DEFINE)

# Now we may create the RPM build tree structure
set(CPACK_RPM_ROOTDIR "${CPACK_TOPLEVEL_DIRECTORY}")
message(STATUS "CPackRPM:Debug: Using CPACK_RPM_ROOTDIR=${CPACK_RPM_ROOTDIR}")
# Prepare RPM build tree
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR})
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/tmp)
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/BUILD)
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/RPMS)
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SOURCES)
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SPECS)
file(MAKE_DIRECTORY ${CPACK_RPM_ROOTDIR}/SRPMS)

#set(CPACK_RPM_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}-${CPACK_RPM_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}-${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")
set(CPACK_RPM_FILE_NAME "${CPACK_OUTPUT_FILE_NAME}")
# it seems rpmbuild can't handle spaces in the path
# neither escaping (as below) nor putting quotes around the path seem to help
#string(REGEX REPLACE " " "\\\\ " CPACK_RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")
set(CPACK_RPM_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}")

# Use files tree to construct files command (spec file)
# We should not forget to include symlinks (thus -o -type l)
# We should include directory as well (thus -type d)
#   but not the main local dir "." (thus -a -not -name ".")
# We must remove the './' due to the local search and escape the
# file name by enclosing it between double quotes (thus the sed)
# Then we must authorize any man pages extension (adding * at the end)
# because rpmbuild may automatically compress those files
execute_process(COMMAND find . -type f -o -type l -o (-type d -a -not -name ".")
                COMMAND sed s:.*/man.*/.*:&*:
                COMMAND sed s/\\.\\\(.*\\\)/\"\\1\"/
                WORKING_DIRECTORY "${WDIR}"
                OUTPUT_VARIABLE CPACK_RPM_INSTALL_FILES)

# In component case, put CPACK_ABSOLUTE_DESTINATION_FILES_<COMPONENT>
#                   into CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL
#         otherwise, put CPACK_ABSOLUTE_DESTINATION_FILES
# This must be done BEFORE the CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL handling
if(CPACK_RPM_PACKAGE_COMPONENT)
  if(CPACK_ABSOLUTE_DESTINATION_FILES)
   set(COMPONENT_FILES_TAG "CPACK_ABSOLUTE_DESTINATION_FILES_${CPACK_RPM_PACKAGE_COMPONENT}")
   set(CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL "${${COMPONENT_FILES_TAG}}")
   if(CPACK_RPM_PACKAGE_DEBUG)
     message("CPackRPM:Debug: Handling Absolute Destination Files: <${CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL}>")
     message("CPackRPM:Debug: in component = ${CPACK_RPM_PACKAGE_COMPONENT}")
   endif(CPACK_RPM_PACKAGE_DEBUG)
  endif()
else()
  if(CPACK_ABSOLUTE_DESTINATION_FILES)
    set(CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL "${CPACK_ABSOLUTE_DESTINATION_FILES}")
  endif()
endif()

# In component case, set CPACK_RPM_USER_FILELIST_INTERNAL with CPACK_RPM_<COMPONENT>_USER_FILELIST.
if(CPACK_RPM_PACKAGE_COMPONENT)
  if(CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_USER_FILELIST)
    set(CPACK_RPM_USER_FILELIST_INTERNAL ${CPACK_RPM_${CPACK_RPM_PACKAGE_COMPONENT}_USER_FILELIST})
    if(CPACK_RPM_PACKAGE_DEBUG)
      message("CPackRPM:Debug: Handling User Filelist: <${CPACK_RPM_USER_FILELIST_INTERNAL}>")
      message("CPackRPM:Debug: in component = ${CPACK_RPM_PACKAGE_COMPONENT}")
    endif(CPACK_RPM_PACKAGE_DEBUG)
  else()
    set(CPACK_RPM_USER_FILELIST_INTERNAL "")
  endif()
else()
  if(CPACK_RPM_USER_FILELIST)
    set(CPACK_RPM_USER_FILELIST_INTERNAL "${CPACK_RPM_USER_FILELIST}")
  else()
    set(CPACK_RPM_USER_FILELIST_INTERNAL "")
  endif()
endif()

# Handle user specified file line list in CPACK_RPM_USER_FILELIST_INTERNAL
# Remove those files from CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL
#                      or CPACK_RPM_INSTALL_FILES,
# hence it must be done before these auto-generated lists are processed.
if(CPACK_RPM_USER_FILELIST_INTERNAL)
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: Handling User Filelist: <${CPACK_RPM_USER_FILELIST_INTERNAL}>")
  endif(CPACK_RPM_PACKAGE_DEBUG)

  # Create CMake list from CPACK_RPM_INSTALL_FILES
  string(STRIP "${CPACK_RPM_INSTALL_FILES}" CPACK_RPM_INSTALL_FILES_LIST)
  string(REPLACE "\n" ";" CPACK_RPM_INSTALL_FILES_LIST
                          "${CPACK_RPM_INSTALL_FILES_LIST}")
  string(REPLACE "\"" "" CPACK_RPM_INSTALL_FILES_LIST
                          "${CPACK_RPM_INSTALL_FILES_LIST}")

  set(CPACK_RPM_USER_INSTALL_FILES "")
  foreach(F IN LISTS CPACK_RPM_USER_FILELIST_INTERNAL)
    string(REGEX REPLACE "%[A-Za-z\(\)]* " "" F_PATH ${F})
    string(REGEX MATCH "%[A-Za-z\(\)]*" F_PREFIX ${F})

    if(F_PREFIX)
        set(F_PREFIX "${F_PREFIX} ")
    endif()
    # Rebuild the user list file
    set(CPACK_RPM_USER_INSTALL_FILES "${CPACK_RPM_USER_INSTALL_FILES}${F_PREFIX}\"${F_PATH}\"\n")

    # Remove from CPACK_RPM_INSTALL_FILES and CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL
    list(REMOVE_ITEM CPACK_RPM_INSTALL_FILES_LIST ${F_PATH})
    list(REMOVE_ITEM CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL ${F_PATH})

  endforeach()

  # Rebuild CPACK_RPM_INSTALL_FILES
  set(CPACK_RPM_INSTALL_FILES "")
  foreach(F IN LISTS CPACK_RPM_INSTALL_FILES_LIST)
    set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}\"${F}\"\n")
  endforeach(F)
else()
  set(CPACK_RPM_USER_INSTALL_FILES "")
endif()

if (CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL)
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: Handling Absolute Destination Files: ${CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL}")
  endif(CPACK_RPM_PACKAGE_DEBUG)
  # Remove trailing space
  string(STRIP "${CPACK_RPM_INSTALL_FILES}" CPACK_RPM_INSTALL_FILES_LIST)
  # Transform endline separated - string into CMake List
  string(REPLACE "\n" ";" CPACK_RPM_INSTALL_FILES_LIST "${CPACK_RPM_INSTALL_FILES_LIST}")
  # Remove unecessary quotes
  string(REPLACE "\"" "" CPACK_RPM_INSTALL_FILES_LIST "${CPACK_RPM_INSTALL_FILES_LIST}")
  # Remove ABSOLUTE install file from INSTALL FILE LIST
  list(REMOVE_ITEM CPACK_RPM_INSTALL_FILES_LIST ${CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL})
  # Rebuild INSTALL_FILES
  set(CPACK_RPM_INSTALL_FILES "")
  foreach(F IN LISTS CPACK_RPM_INSTALL_FILES_LIST)
    set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}\"${F}\"\n")
  endforeach(F)
  # Build ABSOLUTE_INSTALL_FILES
  set(CPACK_RPM_ABSOLUTE_INSTALL_FILES "")
  foreach(F IN LISTS CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL)
    set(CPACK_RPM_ABSOLUTE_INSTALL_FILES "${CPACK_RPM_ABSOLUTE_INSTALL_FILES}%config \"${F}\"\n")
  endforeach(F)
  if(CPACK_RPM_PACKAGE_DEBUG)
    message("CPackRPM:Debug: CPACK_RPM_ABSOLUTE_INSTALL_FILES=${CPACK_RPM_ABSOLUTE_INSTALL_FILES}")
    message("CPackRPM:Debug: CPACK_RPM_INSTALL_FILES=${CPACK_RPM_INSTALL_FILES}")
  endif(CPACK_RPM_PACKAGE_DEBUG)
else()
  # reset vars in order to avoid leakage of value(s) from one component to another
  set(CPACK_RPM_ABSOLUTE_INSTALL_FILES "")
endif(CPACK_ABSOLUTE_DESTINATION_FILES_INTERNAL)

# Prepend directories in ${CPACK_RPM_INSTALL_FILES} with %dir
# This is necessary to avoid duplicate files since rpmbuild do
# recursion on its own when encountering a pathname which is a directory
# which is not flagged as %dir
string(STRIP "${CPACK_RPM_INSTALL_FILES}" CPACK_RPM_INSTALL_FILES_LIST)
string(REPLACE "\n" ";" CPACK_RPM_INSTALL_FILES_LIST
                        "${CPACK_RPM_INSTALL_FILES_LIST}")
string(REPLACE "\"" "" CPACK_RPM_INSTALL_FILES_LIST
                        "${CPACK_RPM_INSTALL_FILES_LIST}")
set(CPACK_RPM_INSTALL_FILES "")
foreach(F IN LISTS CPACK_RPM_INSTALL_FILES_LIST)
  if(IS_DIRECTORY "${WDIR}/${F}")
    set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}%dir \"${F}\"\n")
  else()
    set(CPACK_RPM_INSTALL_FILES "${CPACK_RPM_INSTALL_FILES}\"${F}\"\n")
  endif()
endforeach(F)
set(CPACK_RPM_INSTALL_FILES_LIST "")

# The name of the final spec file to be used by rpmbuild
set(CPACK_RPM_BINARY_SPECFILE "${CPACK_RPM_ROOTDIR}/SPECS/${CPACK_RPM_PACKAGE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.spec")

# Print out some debug information if we were asked for that
if(CPACK_RPM_PACKAGE_DEBUG)
   message("CPackRPM:Debug: CPACK_TOPLEVEL_DIRECTORY          = ${CPACK_TOPLEVEL_DIRECTORY}")
   message("CPackRPM:Debug: CPACK_TOPLEVEL_TAG                = ${CPACK_TOPLEVEL_TAG}")
   message("CPackRPM:Debug: CPACK_TEMPORARY_DIRECTORY         = ${CPACK_TEMPORARY_DIRECTORY}")
   message("CPackRPM:Debug: CPACK_OUTPUT_FILE_NAME            = ${CPACK_OUTPUT_FILE_NAME}")
   message("CPackRPM:Debug: CPACK_OUTPUT_FILE_PATH            = ${CPACK_OUTPUT_FILE_PATH}")
   message("CPackRPM:Debug: CPACK_PACKAGE_FILE_NAME           = ${CPACK_PACKAGE_FILE_NAME}")
   message("CPackRPM:Debug: CPACK_RPM_BINARY_SPECFILE         = ${CPACK_RPM_BINARY_SPECFILE}")
   message("CPackRPM:Debug: CPACK_PACKAGE_INSTALL_DIRECTORY   = ${CPACK_PACKAGE_INSTALL_DIRECTORY}")
   message("CPackRPM:Debug: CPACK_TEMPORARY_PACKAGE_FILE_NAME = ${CPACK_TEMPORARY_PACKAGE_FILE_NAME}")
endif(CPACK_RPM_PACKAGE_DEBUG)

# USER generated spec file handling.
# We should generate a spec file template:
#  - either because the user asked for it : CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE
#  - or the user did not provide one : NOT CPACK_RPM_USER_BINARY_SPECFILE
#
if(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE OR NOT CPACK_RPM_USER_BINARY_SPECFILE)
   file(WRITE ${CPACK_RPM_BINARY_SPECFILE}.in
      "# -*- rpm-spec -*-
BuildRoot:      \@CPACK_RPM_DIRECTORY\@/\@CPACK_PACKAGE_FILE_NAME\@\@CPACK_RPM_PACKAGE_COMPONENT_PART_PATH\@
Summary:        \@CPACK_RPM_PACKAGE_SUMMARY\@
Name:           \@CPACK_RPM_PACKAGE_NAME\@\@CPACK_RPM_PACKAGE_COMPONENT_PART_NAME\@
Version:        \@CPACK_RPM_PACKAGE_VERSION\@
Release:        \@CPACK_RPM_PACKAGE_RELEASE\@
License:        \@CPACK_RPM_PACKAGE_LICENSE\@
Group:          \@CPACK_RPM_PACKAGE_GROUP\@
Vendor:         \@CPACK_RPM_PACKAGE_VENDOR\@
\@TMP_RPM_URL\@
\@TMP_RPM_REQUIRES\@
\@TMP_RPM_PROVIDES\@
\@TMP_RPM_OBSOLETES\@
\@TMP_RPM_CONFLICTS\@
\@TMP_RPM_AUTOPROV\@
\@TMP_RPM_AUTOREQ\@
\@TMP_RPM_AUTOREQPROV\@
\@TMP_RPM_BUILDARCH\@
\@TMP_RPM_PREFIX\@

%define _rpmdir \@CPACK_RPM_DIRECTORY\@
%define _rpmfilename \@CPACK_RPM_FILE_NAME\@
%define _unpackaged_files_terminate_build 0
%define _topdir \@CPACK_RPM_DIRECTORY\@
\@TMP_RPM_SPEC_INSTALL_POST\@
\@CPACK_RPM_SPEC_MORE_DEFINE\@
\@CPACK_RPM_COMPRESSION_TYPE_TMP\@

%description
\@CPACK_RPM_PACKAGE_DESCRIPTION\@

# This is a shortcutted spec file generated by CMake RPM generator
# we skip _install step because CPack does that for us.
# We do only save CPack installed tree in _prepr
# and then restore it in build.
%prep
mv $RPM_BUILD_ROOT \"\@CPACK_TOPLEVEL_DIRECTORY\@/tmpBBroot\"

#p build

%install
if [ -e $RPM_BUILD_ROOT ];
then
  rm -rf $RPM_BUILD_ROOT
fi
mv \"\@CPACK_TOPLEVEL_DIRECTORY\@/tmpBBroot\" $RPM_BUILD_ROOT

%clean

%post
\@CPACK_RPM_SPEC_POSTINSTALL\@

%postun
\@CPACK_RPM_SPEC_POSTUNINSTALL\@

%pre
\@CPACK_RPM_SPEC_PREINSTALL\@

%preun
\@CPACK_RPM_SPEC_PREUNINSTALL\@

%files
%defattr(-,root,root,-)
${CPACK_RPM_INSTALL_FILES}
${CPACK_RPM_ABSOLUTE_INSTALL_FILES}
${CPACK_RPM_USER_INSTALL_FILES}

%changelog
\@CPACK_RPM_SPEC_CHANGELOG\@
")
  # Stop here if we were asked to only generate a template USER spec file
  # The generated file may then be used as a template by user who wants
  # to customize their own spec file.
  if(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE)
     message(FATAL_ERROR "CPackRPM: STOP here Generated USER binary spec file templare is: ${CPACK_RPM_BINARY_SPECFILE}.in")
  endif(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE)
endif(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE OR NOT CPACK_RPM_USER_BINARY_SPECFILE)

# After that we may either use a user provided spec file
# or generate one using appropriate variables value.
if(CPACK_RPM_USER_BINARY_SPECFILE)
  # User may have specified SPECFILE just use it
  message("CPackRPM: Will use USER specified spec file: ${CPACK_RPM_USER_BINARY_SPECFILE}")
  # The user provided file is processed for @var replacement
  configure_file(${CPACK_RPM_USER_BINARY_SPECFILE} ${CPACK_RPM_BINARY_SPECFILE} @ONLY)
else(CPACK_RPM_USER_BINARY_SPECFILE)
  # No User specified spec file, will use the generated spec file
  message("CPackRPM: Will use GENERATED spec file: ${CPACK_RPM_BINARY_SPECFILE}")
  # Note the just created file is processed for @var replacement
  configure_file(${CPACK_RPM_BINARY_SPECFILE}.in ${CPACK_RPM_BINARY_SPECFILE} @ONLY)
endif(CPACK_RPM_USER_BINARY_SPECFILE)

if(RPMBUILD_EXECUTABLE)
  # Now call rpmbuild using the SPECFILE
  execute_process(
    COMMAND "${RPMBUILD_EXECUTABLE}" -bb
            --define "_topdir ${CPACK_RPM_DIRECTORY}"
            --buildroot "${CPACK_RPM_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_PATH}"
            "${CPACK_RPM_BINARY_SPECFILE}"
    WORKING_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_RPM_PACKAGE_COMPONENT_PART_PATH}"
    RESULT_VARIABLE CPACK_RPMBUILD_EXEC_RESULT
    ERROR_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.err"
    OUTPUT_FILE "${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.out")
  if(CPACK_RPM_PACKAGE_DEBUG OR CPACK_RPMBUILD_EXEC_RESULT)
    file(READ ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.err RPMBUILDERR)
    file(READ ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.out RPMBUILDOUT)
    message("CPackRPM:Debug: You may consult rpmbuild logs in: ")
    message("CPackRPM:Debug:    - ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.err")
    message("CPackRPM:Debug: *** ${RPMBUILDERR} ***")
    message("CPackRPM:Debug:    - ${CPACK_TOPLEVEL_DIRECTORY}/rpmbuild${CPACK_RPM_PACKAGE_COMPONENT_PART_NAME}.out")
    message("CPackRPM:Debug: *** ${RPMBUILDERR} ***")
  endif(CPACK_RPM_PACKAGE_DEBUG OR CPACK_RPMBUILD_EXEC_RESULT)
else(RPMBUILD_EXECUTABLE)
  if(ALIEN_EXECUTABLE)
    message(FATAL_ERROR "RPM packaging through alien not done (yet)")
  endif(ALIEN_EXECUTABLE)
endif(RPMBUILD_EXECUTABLE)
