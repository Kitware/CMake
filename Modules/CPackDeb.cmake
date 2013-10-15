#.rst:
# CPackDeb
# --------
#
# The builtin (binary) CPack Deb generator (Unix only)
#
# #section Variables specific to CPack Debian (DEB) generator #end
# #module CPackDeb may be used to create Deb package using CPack.
# CPackDeb is a CPack generator thus it uses the CPACK_XXX variables
# used by CPack : http://www.cmake.org/Wiki/CMake:CPackConfiguration.
# CPackDeb generator should work on any linux host but it will produce
# better deb package when Debian specific tools 'dpkg-xxx' are usable on
# the build system.
#
# CPackDeb has specific features which are controlled by the specifics
# CPACK_DEBIAN_XXX variables.You'll find a detailed usage on the wiki:
#
# ::
#
#   http://www.cmake.org/Wiki/CMake:CPackPackageGenerators#DEB_.28UNIX_only.29
#
# However as a handy reminder here comes the list of specific variables:
# #end
#
# #variable CPACK_DEBIAN_PACKAGE_NAME
#
# ::
#
#      Mandatory : YES
#      Default   : CPACK_PACKAGE_NAME (lower case)
#      The debian package summary
#
# #end #variable CPACK_DEBIAN_PACKAGE_VERSION
#
# ::
#
#      Mandatory : YES
#      Default   : CPACK_PACKAGE_VERSION
#      The debian package version
#
# #end #variable CPACK_DEBIAN_PACKAGE_ARCHITECTURE
#
# ::
#
#      Mandatory : YES
#      Default   : Output of dpkg --print-architecture (or i386 if dpkg is not found)
#      The debian package architecture
#
# #end #variable CPACK_DEBIAN_PACKAGE_DEPENDS
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      May be used to set deb dependencies.
#
# #end #variable CPACK_DEBIAN_PACKAGE_MAINTAINER
#
# ::
#
#      Mandatory : YES
#      Default   : CPACK_PACKAGE_CONTACT
#      The debian package maintainer
#
# #end #variable CPACK_DEBIAN_PACKAGE_DESCRIPTION
#
# ::
#
#      Mandatory : YES
#      Default   : CPACK_PACKAGE_DESCRIPTION_SUMMARY
#      The debian package description
#
# #end #variable CPACK_DEBIAN_PACKAGE_SECTION
#
# ::
#
#      Mandatory : YES
#      Default   : 'devel'
#      The debian package section
#
# #end #variable CPACK_DEBIAN_PACKAGE_PRIORITY
#
# ::
#
#      Mandatory : YES
#      Default   : 'optional'
#      The debian package priority
#
# #end #variable CPACK_DEBIAN_PACKAGE_HOMEPAGE
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      The URL of the web site for this package, preferably (when applicable) the
#      site from which the original source can be obtained and any additional
#      upstream documentation or information may be found.
#      The content of this field is a simple URL without any surrounding
#      characters such as <>.
#
# #end #variable CPACK_DEBIAN_PACKAGE_SHLIBDEPS
#
# ::
#
#      Mandatory : NO
#      Default   : OFF
#      May be set to ON in order to use dpkg-shlibdeps to generate
#      better package dependency list.
#      You may need set CMAKE_INSTALL_RPATH toi appropriate value
#      if you use this feature, because if you don't dpkg-shlibdeps
#      may fail to find your own shared libs.
#      See http://www.cmake.org/Wiki/CMake_RPATH_handling.
#
# #end #variable CPACK_DEBIAN_PACKAGE_DEBUG
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      May be set when invoking cpack in order to trace debug information
#      during CPackDeb run.
#
# #end #variable CPACK_DEBIAN_PACKAGE_PREDEPENDS
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      This field is like Depends, except that it also forces dpkg to complete installation of
#      the packages named before even starting the installation of the package which declares
#      the pre-dependency.
#
# #end #variable CPACK_DEBIAN_PACKAGE_ENHANCES
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      This field is similar to Suggests but works in the opposite direction.
#      It is used to declare that a package can enhance the functionality of another package.
#
# #end #variable CPACK_DEBIAN_PACKAGE_BREAKS
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      When one binary package declares that it breaks another, dpkg will refuse to allow the
#      package which declares Breaks be installed unless the broken package is deconfigured first,
#      and it will refuse to allow the broken package to be reconfigured.
#
# #end #variable CPACK_DEBIAN_PACKAGE_CONFLICTS
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      When one binary package declares a conflict with another using a Conflicts field,
#      dpkg will refuse to allow them to be installed on the system at the same time.
#
# #end #variable CPACK_DEBIAN_PACKAGE_PROVIDES
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      A virtual package is one which appears in the Provides control field of another package.
#
# #end #variable CPACK_DEBIAN_PACKAGE_REPLACES
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      Packages can declare in their control file that they should overwrite
#      files in certain other packages, or completely replace other packages.
#
# #end #variable CPACK_DEBIAN_PACKAGE_RECOMMENDS
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      Allows packages to declare a strong, but not absolute, dependency on other packages.
#
# #end #variable CPACK_DEBIAN_PACKAGE_SUGGESTS
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      see http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#      Allows packages to declare a suggested package install grouping.
#
# #end #variable CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#
# ::
#
#      Mandatory : NO
#      Default   : -
#      This variable allow advanced user to add custom script to the control.tar.gz
#      Typical usage is for conffiles, postinst, postrm, prerm.
#      Usage: set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#             "${CMAKE_CURRENT_SOURCE_DIR/prerm;${CMAKE_CURRENT_SOURCE_DIR}/postrm")
#
# #end


#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
# Copyright 2007-2009 Mathieu Malaterre <mathieu.malaterre@gmail.com>
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

# CPack script for creating Debian package
# Author: Mathieu Malaterre
#
# http://wiki.debian.org/HowToPackageForDebian

if(CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CPackDeb.cmake may only be used by CPack internally.")
endif()

if(NOT UNIX)
  message(FATAL_ERROR "CPackDeb.cmake may only be used under UNIX.")
endif()

# CPACK_DEBIAN_PACKAGE_SHLIBDEPS
# If specify OFF, only user depends are used
if(NOT DEFINED CPACK_DEBIAN_PACKAGE_SHLIBDEPS)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
endif()

find_program(FAKEROOT_EXECUTABLE fakeroot)
if(FAKEROOT_EXECUTABLE)
  set(CPACK_DEBIAN_FAKEROOT_EXECUTABLE ${FAKEROOT_EXECUTABLE})
endif()

if(CPACK_DEBIAN_PACKAGE_SHLIBDEPS)
  # dpkg-shlibdeps is a Debian utility for generating dependency list
  find_program(SHLIBDEPS_EXECUTABLE dpkg-shlibdeps)

  # Check version of the dpkg-shlibdeps tool using CPackRPM method
  if(SHLIBDEPS_EXECUTABLE)
    execute_process(COMMAND ${SHLIBDEPS_EXECUTABLE} --version
      OUTPUT_VARIABLE _TMP_VERSION
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "dpkg-shlibdeps version ([0-9]+\\.[0-9]+\\.[0-9]+)"
      SHLIBDEPS_EXECUTABLE_VERSION
      ${_TMP_VERSION})
    set(SHLIBDEPS_EXECUTABLE_VERSION "${CMAKE_MATCH_1}")
    if(CPACK_DEBIAN_PACKAGE_DEBUG)
      message( "CPackDeb Debug: dpkg-shlibdeps version is <${SHLIBDEPS_EXECUTABLE_VERSION}>")
    endif()

    # Generating binary list - Get type of all install files
    execute_process(COMMAND find -type f
      COMMAND xargs file
      WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}"
      OUTPUT_VARIABLE CPACK_DEB_INSTALL_FILES)

    # Convert to CMake list
    string(REGEX REPLACE "\n" ";" CPACK_DEB_INSTALL_FILES ${CPACK_DEB_INSTALL_FILES})

    # Only dynamically linked ELF files are included
    # Extract only file name infront of ":"
    foreach ( _FILE ${CPACK_DEB_INSTALL_FILES})
      if ( ${_FILE} MATCHES "ELF.*dynamically linked")
         string(REGEX MATCH "(^.*):" _FILE_NAME ${_FILE})
         list(APPEND CPACK_DEB_BINARY_FILES ${CMAKE_MATCH_1})
      endif()
    endforeach()

    message( "CPackDeb: - Generating dependency list")

    # Create blank control file for running dpkg-shlibdeps
    # There might be some other way to invoke dpkg-shlibdeps without creating this file
    # but standard debian package should not have anything that can collide with this file or directory
    file(MAKE_DIRECTORY ${CPACK_TEMPORARY_DIRECTORY}/debian)
    file(WRITE ${CPACK_TEMPORARY_DIRECTORY}/debian/control "")

    # Execute dpkg-shlibdeps
    # --ignore-missing-info : allow dpkg-shlibdeps to run even if some libs do not belong to a package
    # -O : print to STDOUT
    execute_process(COMMAND ${SHLIBDEPS_EXECUTABLE} --ignore-missing-info -O ${CPACK_DEB_BINARY_FILES}
      WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}"
      OUTPUT_VARIABLE SHLIBDEPS_OUTPUT
      RESULT_VARIABLE SHLIBDEPS_RESULT
      ERROR_VARIABLE SHLIBDEPS_ERROR
      OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(CPACK_DEBIAN_PACKAGE_DEBUG)
      # dpkg-shlibdeps will throw some warnings if some input files are not binary
      message( "CPackDeb Debug: dpkg-shlibdeps warnings \n${SHLIBDEPS_ERROR}")
    endif()
    if (NOT SHLIBDEPS_RESULT EQUAL 0)
      message (FATAL_ERROR "CPackDeb: dpkg-shlibdeps: ${SHLIBDEPS_ERROR}")
    endif ()

    #Get rid of prefix generated by dpkg-shlibdeps
    string (REGEX REPLACE "^.*Depends=" "" CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS ${SHLIBDEPS_OUTPUT})

    if(CPACK_DEBIAN_PACKAGE_DEBUG)
      message( "CPackDeb Debug: Found dependency: ${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS}")
    endif()

    # Remove blank control file
    # Might not be safe if package actual contain file or directory named debian
    file(REMOVE_RECURSE "${CPACK_TEMPORARY_DIRECTORY}/debian")

    # Append user depend if set
    if (CPACK_DEBIAN_PACKAGE_DEPENDS)
      set (CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS}, ${CPACK_DEBIAN_PACKAGE_DEPENDS}")
    else ()
      set (CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS}")
    endif ()

  else ()
    if(CPACK_DEBIAN_PACKAGE_DEBUG)
      message( "CPackDeb Debug: Using only user-provided depends because dpkg-shlibdeps is not found.")
    endif()
  endif()

else ()
  if(CPACK_DEBIAN_PACKAGE_DEBUG)
    message( "CPackDeb Debug: Using only user-provided depends")
  endif()
endif()

# Let's define the control file found in debian package:

# Binary package:
# http://www.debian.org/doc/debian-policy/ch-controlfields.html#s-binarycontrolfiles

# DEBIAN/control
# debian policy enforce lower case for package name
# Package: (mandatory)
if(NOT CPACK_DEBIAN_PACKAGE_NAME)
  string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_DEBIAN_PACKAGE_NAME)
endif()

# Version: (mandatory)
if(NOT CPACK_DEBIAN_PACKAGE_VERSION)
  if(NOT CPACK_PACKAGE_VERSION)
    message(FATAL_ERROR "CPackDeb: Debian package requires a package version")
  endif()
  set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
endif()

# Architecture: (mandatory)
if(NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
  # There is no such thing as i686 architecture on debian, you should use i386 instead
  # $ dpkg --print-architecture
  find_program(DPKG_CMD dpkg)
  if(NOT DPKG_CMD)
    message(STATUS "CPackDeb: Can not find dpkg in your path, default to i386.")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
  endif()
  execute_process(COMMAND "${DPKG_CMD}" --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# have a look at get_property(result GLOBAL PROPERTY ENABLED_FEATURES),
# this returns the successful find_package() calls, maybe this can help
# Depends:
# You should set: DEBIAN_PACKAGE_DEPENDS
# TODO: automate 'objdump -p | grep NEEDED'
if(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
  message(STATUS "CPACK_DEBIAN_PACKAGE_DEPENDS not set, the package will have no dependencies.")
endif()

# Maintainer: (mandatory)
if(NOT CPACK_DEBIAN_PACKAGE_MAINTAINER)
  if(NOT CPACK_PACKAGE_CONTACT)
    message(FATAL_ERROR "CPackDeb: Debian package requires a maintainer for a package, set CPACK_PACKAGE_CONTACT or CPACK_DEBIAN_PACKAGE_MAINTAINER")
  endif()
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
endif()

# Description: (mandatory)
if(NOT CPACK_DEBIAN_PACKAGE_DESCRIPTION)
  if(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
    message(FATAL_ERROR "CPackDeb: Debian package requires a summary for a package, set CPACK_PACKAGE_DESCRIPTION_SUMMARY or CPACK_DEBIAN_PACKAGE_DESCRIPTION")
  endif()
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
endif()

# Section: (recommended)
if(NOT CPACK_DEBIAN_PACKAGE_SECTION)
  set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
endif()

# Priority: (recommended)
if(NOT CPACK_DEBIAN_PACKAGE_PRIORITY)
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
endif()

# Recommends:
# You should set: CPACK_DEBIAN_PACKAGE_RECOMMENDS

# Suggests:
# You should set: CPACK_DEBIAN_PACKAGE_SUGGESTS

# CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
# This variable allow advanced user to add custom script to the control.tar.gz (inside the .deb archive)
# Typical examples are:
# - conffiles
# - postinst
# - postrm
# - prerm"
# Usage:
# set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#    "${CMAKE_CURRENT_SOURCE_DIR/prerm;${CMAKE_CURRENT_SOURCE_DIR}/postrm")

# Are we packaging components ?
if(CPACK_DEB_PACKAGE_COMPONENT)
  set(CPACK_DEB_PACKAGE_COMPONENT_PART_NAME "-${CPACK_DEB_PACKAGE_COMPONENT}")
  string(TOLOWER "${CPACK_PACKAGE_NAME}${CPACK_DEB_PACKAGE_COMPONENT_PART_NAME}" CPACK_DEBIAN_PACKAGE_NAME)
else()
  set(CPACK_DEB_PACKAGE_COMPONENT_PART_NAME "")
endif()

set(WDIR "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_DEB_PACKAGE_COMPONENT_PART_PATH}")

# Print out some debug information if we were asked for that
if(CPACK_DEBIAN_PACKAGE_DEBUG)
   message("CPackDeb:Debug: CPACK_TOPLEVEL_DIRECTORY          = ${CPACK_TOPLEVEL_DIRECTORY}")
   message("CPackDeb:Debug: CPACK_TOPLEVEL_TAG                = ${CPACK_TOPLEVEL_TAG}")
   message("CPackDeb:Debug: CPACK_TEMPORARY_DIRECTORY         = ${CPACK_TEMPORARY_DIRECTORY}")
   message("CPackDeb:Debug: CPACK_OUTPUT_FILE_NAME            = ${CPACK_OUTPUT_FILE_NAME}")
   message("CPackDeb:Debug: CPACK_OUTPUT_FILE_PATH            = ${CPACK_OUTPUT_FILE_PATH}")
   message("CPackDeb:Debug: CPACK_PACKAGE_FILE_NAME           = ${CPACK_PACKAGE_FILE_NAME}")
   message("CPackDeb:Debug: CPACK_PACKAGE_INSTALL_DIRECTORY   = ${CPACK_PACKAGE_INSTALL_DIRECTORY}")
   message("CPackDeb:Debug: CPACK_TEMPORARY_PACKAGE_FILE_NAME = ${CPACK_TEMPORARY_PACKAGE_FILE_NAME}")
endif()

# For debian source packages:
# debian/control
# http://www.debian.org/doc/debian-policy/ch-controlfields.html#s-sourcecontrolfiles

# .dsc
# http://www.debian.org/doc/debian-policy/ch-controlfields.html#s-debiansourcecontrolfiles

# Builds-Depends:
#if(NOT CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS)
#  set(CPACK_DEBIAN_PACKAGE_BUILDS_DEPENDS
#    "debhelper (>> 5.0.0), libncurses5-dev, tcl8.4"
#  )
#endif()
