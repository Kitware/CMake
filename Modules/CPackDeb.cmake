#.rst:
# CPackDeb
# --------
#
# The builtin (binary) CPack Deb generator (Unix only)
#
# Variables specific to CPack Debian (DEB) generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# CPackDeb may be used to create Deb package using CPack.
# CPackDeb is a CPack generator thus it uses the ``CPACK_XXX`` variables
# used by CPack : https://cmake.org/Wiki/CMake:CPackConfiguration.
# CPackDeb generator should work on any linux host but it will produce
# better deb package when Debian specific tools 'dpkg-xxx' are usable on
# the build system.
#
# CPackDeb has specific features which are controlled by the specifics
# :code:`CPACK_DEBIAN_XXX` variables.
#
# :code:`CPACK_DEBIAN_<COMPONENT>_XXXX` variables may be used in order to have
# **component** specific values.  Note however that ``<COMPONENT>`` refers to the
# **grouping name** written in upper case. It may be either a component name or
# a component GROUP name.
#
# You'll find a detailed usage on the wiki:
# https://cmake.org/Wiki/CMake:CPackPackageGenerators#DEB_.28UNIX_only.29 .
# However as a handy reminder here comes the list of specific variables:
#
# .. variable:: CPACK_DEBIAN_PACKAGE_NAME
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_NAME
#
#  Set Package control field (variable is automatically transformed to lower
#  case).
#
#  * Mandatory : YES
#  * Default   :
#
#    - :variable:`CPACK_PACKAGE_NAME` for non-component based
#      installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_NAME` suffixed with -<COMPONENT>
#      for component-based installations.
#
#  See https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Source
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_VERSION
#
#  The Debian package version
#
#  * Mandatory : YES
#  * Default   : :variable:`CPACK_PACKAGE_VERSION`
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_ARCHITECTURE
#
#  The Debian package architecture
#
#  * Mandatory : YES
#  * Default   : Output of :code:`dpkg --print-architecture` (or :code:`i386`
#    if :code:`dpkg` is not found)
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_DEPENDS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_DEPENDS
#
#  Sets the Debian dependencies of this package.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_DEPENDS` for component-based
#      installations.
#
#  .. note::
#
#    If :variable:`CPACK_DEBIAN_PACKAGE_SHLIBDEPS` or
#    more specifically :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_SHLIBDEPS`
#    is set for this component, the discovered dependencies will be appended
#    to :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_DEPENDS` instead of
#    :variable:`CPACK_DEBIAN_PACKAGE_DEPENDS`. If
#    :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_DEPENDS` is an empty string,
#    only the automatically discovered dependencies will be set for this
#    component.
#
#  Example::
#
#    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libc6 (< 2.4)")
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_MAINTAINER
#
#  The Debian package maintainer
#
#  * Mandatory : YES
#  * Default   : :code:`CPACK_PACKAGE_CONTACT`
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_DESCRIPTION
#               CPACK_COMPONENT_<COMPONENT>_DESCRIPTION
#
#  The Debian package description
#
#  * Mandatory : YES
#  * Default   :
#
#    - :variable:`CPACK_DEBIAN_PACKAGE_DESCRIPTION` if set or
#    - :variable:`CPACK_PACKAGE_DESCRIPTION_SUMMARY`
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_SECTION
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_SECTION
#
#  Set Section control field e.g. admin, devel, doc, ...
#
#  * Mandatory : YES
#  * Default   : 'devel'
#
#  See https://www.debian.org/doc/debian-policy/ch-archive.html#s-subsections
#
#
# .. variable:: CPACK_DEBIAN_COMPRESSION_TYPE
#
#  The compression used for creating the Debian package.
#  Possible values are: lzma, xz, bzip2 and gzip.
#
#  * Mandatory : YES
#  * Default   : 'gzip'
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_PRIORITY
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_PRIORITY
#
#  Set Priority control field e.g. required, important, standard, optional,
#  extra
#
#  * Mandatory : YES
#  * Default   : 'optional'
#
#  See https://www.debian.org/doc/debian-policy/ch-archive.html#s-priorities
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_HOMEPAGE
#
#  The URL of the web site for this package, preferably (when applicable) the
#  site from which the original source can be obtained and any additional
#  upstream documentation or information may be found.
#
#  * Mandatory : NO
#  * Default   : -
#
#  .. note::
#
#    The content of this field is a simple URL without any surrounding
#    characters such as <>.
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_SHLIBDEPS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_SHLIBDEPS
#
#  May be set to ON in order to use :code:`dpkg-shlibdeps` to generate
#  better package dependency list.
#
#  * Mandatory : NO
#  * Default   :
#
#    - :variable:`CPACK_DEBIAN_PACKAGE_SHLIBDEPS` if set or
#    - OFF
#
#  .. note::
#
#    You may need set :variable:`CMAKE_INSTALL_RPATH` to an appropriate value
#    if you use this feature, because if you don't :code:`dpkg-shlibdeps`
#    may fail to find your own shared libs.
#    See https://cmake.org/Wiki/CMake_RPATH_handling.
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_DEBUG
#
#  May be set when invoking cpack in order to trace debug information
#  during CPackDeb run.
#
#  * Mandatory : NO
#  * Default   : -
#
# .. variable:: CPACK_DEBIAN_PACKAGE_PREDEPENDS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_PREDEPENDS
#
#  Sets the `Pre-Depends` field of the Debian package.
#  Like :variable:`Depends <CPACK_DEBIAN_PACKAGE_DEPENDS>`, except that it
#  also forces :code:`dpkg` to complete installation of the packages named
#  before even starting the installation of the package which declares the
#  pre-dependency.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_PREDEPENDS` for component-based
#      installations.
#
#  See http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#
# .. variable:: CPACK_DEBIAN_PACKAGE_ENHANCES
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_ENHANCES
#
#  Sets the `Enhances` field of the Debian package.
#  Similar to :variable:`Suggests <CPACK_DEBIAN_PACKAGE_SUGGESTS>` but works
#  in the opposite direction: declares that a package can enhance the
#  functionality of another package.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_ENHANCES` for component-based
#      installations.
#
#  See http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#
# .. variable:: CPACK_DEBIAN_PACKAGE_BREAKS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_BREAKS
#
#  Sets the `Breaks` field of the Debian package.
#  When a binary package (P) declares that it breaks other packages (B),
#  :code:`dpkg` will not allow the package (P) which declares `Breaks` be
#  **unpacked** unless the packages that will be broken (B) are deconfigured
#  first.
#  As long as the package (P) is configured, the previously deconfigured
#  packages (B) cannot be reconfigured again.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_BREAKS` for component-based
#      installations.
#
#  See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-breaks
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_CONFLICTS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_CONFLICTS
#
#  Sets the `Conflicts` field of the Debian package.
#  When one binary package declares a conflict with another using a `Conflicts`
#  field, :code:`dpkg` will not allow them to be unpacked on the system at
#  the same time.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_CONFLICTS` for component-based
#      installations.
#
#  See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-conflicts
#
#  .. note::
#
#    This is a stronger restriction than
#    :variable:`Breaks <CPACK_DEBIAN_PACKAGE_BREAKS>`, which prevents the
#    broken package from being configured while the breaking package is in
#    the "Unpacked" state but allows both packages to be unpacked at the same
#    time.
#
# .. variable:: CPACK_DEBIAN_PACKAGE_PROVIDES
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_PROVIDES
#
#  Sets the `Provides` field of the Debian package.
#  A virtual package is one which appears in the `Provides` control field of
#  another package.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_PROVIDES` for component-based
#      installations.
#
#  See https://www.debian.org/doc/debian-policy/ch-relationships.html#s-virtual
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_REPLACES
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_REPLACES
#
#  Sets the `Replaces` field of the Debian package.
#  Packages can declare in their control file that they should overwrite
#  files in certain other packages, or completely replace other packages.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_REPLACES` for component-based
#      installations.
#
#  See http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_RECOMMENDS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_RECOMMENDS
#
#  Sets the `Recommends` field of the Debian package.
#  Allows packages to declare a strong, but not absolute, dependency on other
#  packages.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_RECOMMENDS` for component-based
#      installations.
#
#  See http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_SUGGESTS
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_SUGGESTS
#
#  Sets the `Suggests` field of the Debian package.
#  Allows packages to declare a suggested package install grouping.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_SUGGESTS` for component-based
#      installations.
#
#  See http://www.debian.org/doc/debian-policy/ch-relationships.html#s-binarydeps
#
#
# .. variable:: CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_CONTROL_EXTRA
#
#  This variable allow advanced user to add custom script to the
#  control.tar.gz.
#  Typical usage is for conffiles, postinst, postrm, prerm.
#
#  * Mandatory : NO
#  * Default   : -
#
#  Usage::
#
#   set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#       "${CMAKE_CURRENT_SOURCE_DIR/prerm;${CMAKE_CURRENT_SOURCE_DIR}/postrm")
#
#  .. note::
#
#    The original permissions of the files will be used in the final
#    package unless the variable
#    :variable:`CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION` is set.
#    In particular, the scripts should have the proper executable
#    flag prior to the generation of the package.
#
# .. variable:: CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_CONTROL_STRICT_PERMISSION
#
#  This variable indicates if the Debian policy on control files should be
#  strictly followed.
#
#  * Mandatory : NO
#  * Default   : FALSE
#
#  Usage::
#
#   set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
#
#  .. note::
#
#    This overrides the permissions on the original files, following the rules
#    set by Debian policy
#    https://www.debian.org/doc/debian-policy/ch-files.html#s-permissions-owners
#
# .. variable:: CPACK_DEBIAN_PACKAGE_SOURCE
#               CPACK_DEBIAN_<COMPONENT>_PACKAGE_SOURCE
#
#  Sets the ``Source`` field of the binary Debian package.
#  When the binary package name is not the same as the source package name
#  (in particular when several components/binaries are generated from one
#  source) the source from which the binary has been generated should be
#  indicated with the field ``Source``.
#
#  * Mandatory : NO
#  * Default   :
#
#    - An empty string for non-component based installations
#    - :variable:`CPACK_DEBIAN_PACKAGE_SOURCE` for component-based
#      installations.
#
#  See https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Source
#
#  .. note::
#
#    This value is not interpreted. It is possible to pass an optional
#    revision number of the referenced source package as well.

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

function(cpack_deb_prepare_package_vars)
  # CPACK_DEBIAN_PACKAGE_SHLIBDEPS
  # If specify OFF, only user depends are used
  if(NOT DEFINED CPACK_DEBIAN_PACKAGE_SHLIBDEPS)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)
  endif()

  set(WDIR "${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_DEB_PACKAGE_COMPONENT_PART_PATH}")

  # per component automatic discover: some of the component might not have
  # binaries.
  if(CPACK_DEB_PACKAGE_COMPONENT)
    string(TOUPPER "${CPACK_DEB_PACKAGE_COMPONENT}" _local_component_name)
    set(_component_shlibdeps_var "CPACK_DEBIAN_${_local_component_name}_PACKAGE_SHLIBDEPS")

    # if set, overrides the global configuration
    if(DEFINED ${_component_shlibdeps_var})
      set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS "${${_component_shlibdeps_var}}")
      if(CPACK_DEBIAN_PACKAGE_DEBUG)
        message("CPackDeb Debug: component '${CPACK_DEB_PACKAGE_COMPONENT}' dpkg-shlibdeps set to ${CPACK_DEBIAN_PACKAGE_SHLIBDEPS}")
      endif()
    endif()
  endif()

  if(CPACK_DEBIAN_PACKAGE_SHLIBDEPS)
    # dpkg-shlibdeps is a Debian utility for generating dependency list
    find_program(SHLIBDEPS_EXECUTABLE dpkg-shlibdeps)

    if(SHLIBDEPS_EXECUTABLE)
      # Check version of the dpkg-shlibdeps tool using CPackRPM method
      execute_process(COMMAND env LC_ALL=C ${SHLIBDEPS_EXECUTABLE} --version
        OUTPUT_VARIABLE _TMP_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      if(_TMP_VERSION MATCHES "dpkg-shlibdeps version ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(SHLIBDEPS_EXECUTABLE_VERSION "${CMAKE_MATCH_1}")
      else()
        set(SHLIBDEPS_EXECUTABLE_VERSION "")
      endif()

      if(CPACK_DEBIAN_PACKAGE_DEBUG)
        message("CPackDeb Debug: dpkg-shlibdeps --version output is '${_TMP_VERSION}'")
        message("CPackDeb Debug: dpkg-shlibdeps version is <${SHLIBDEPS_EXECUTABLE_VERSION}>")
      endif()

      # Generating binary list - Get type of all install files
      cmake_policy(PUSH)
        # Tell file(GLOB_RECURSE) not to follow directory symlinks
        # even if the project does not set this policy to NEW.
        cmake_policy(SET CMP0009 NEW)
        file(GLOB_RECURSE FILE_PATHS_ LIST_DIRECTORIES false RELATIVE "${WDIR}" "${WDIR}/*")
      cmake_policy(POP)

      # get file info so that we can determine if file is executable or not
      unset(CPACK_DEB_INSTALL_FILES)
      foreach(FILE_ IN LISTS FILE_PATHS_)
        execute_process(COMMAND file "./${FILE_}"
          WORKING_DIRECTORY "${WDIR}"
          OUTPUT_VARIABLE INSTALL_FILE_)
        list(APPEND CPACK_DEB_INSTALL_FILES "${INSTALL_FILE_}")
      endforeach()

      # Only dynamically linked ELF files are included
      # Extract only file name infront of ":"
      foreach(_FILE ${CPACK_DEB_INSTALL_FILES})
        if( ${_FILE} MATCHES "ELF.*dynamically linked")
           string(REGEX MATCH "(^.*):" _FILE_NAME "${_FILE}")
           list(APPEND CPACK_DEB_BINARY_FILES "${CMAKE_MATCH_1}")
           set(CONTAINS_EXECUTABLE_FILES_ TRUE)
        endif()
      endforeach()

      if(CONTAINS_EXECUTABLE_FILES_)
        message("CPackDeb: - Generating dependency list")

        # Create blank control file for running dpkg-shlibdeps
        # There might be some other way to invoke dpkg-shlibdeps without creating this file
        # but standard debian package should not have anything that can collide with this file or directory
        file(MAKE_DIRECTORY ${CPACK_TEMPORARY_DIRECTORY}/debian)
        file(WRITE ${CPACK_TEMPORARY_DIRECTORY}/debian/control "")

        # Add --ignore-missing-info if the tool supports it
        execute_process(COMMAND env LC_ALL=C ${SHLIBDEPS_EXECUTABLE} --help
          OUTPUT_VARIABLE _TMP_HELP
          ERROR_QUIET
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(_TMP_HELP MATCHES "--ignore-missing-info")
          set(IGNORE_MISSING_INFO_FLAG "--ignore-missing-info")
        endif()

        # Execute dpkg-shlibdeps
        # --ignore-missing-info : allow dpkg-shlibdeps to run even if some libs do not belong to a package
        # -O : print to STDOUT
        execute_process(COMMAND ${SHLIBDEPS_EXECUTABLE} ${IGNORE_MISSING_INFO_FLAG} -O ${CPACK_DEB_BINARY_FILES}
          WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}"
          OUTPUT_VARIABLE SHLIBDEPS_OUTPUT
          RESULT_VARIABLE SHLIBDEPS_RESULT
          ERROR_VARIABLE SHLIBDEPS_ERROR
          OUTPUT_STRIP_TRAILING_WHITESPACE )
        if(CPACK_DEBIAN_PACKAGE_DEBUG)
          # dpkg-shlibdeps will throw some warnings if some input files are not binary
          message( "CPackDeb Debug: dpkg-shlibdeps warnings \n${SHLIBDEPS_ERROR}")
        endif()
        if(NOT SHLIBDEPS_RESULT EQUAL 0)
          message (FATAL_ERROR "CPackDeb: dpkg-shlibdeps: '${SHLIBDEPS_ERROR}';\n"
              "executed command: '${SHLIBDEPS_EXECUTABLE} ${IGNORE_MISSING_INFO_FLAG} -O ${CPACK_DEB_BINARY_FILES}';\n"
              "found files: '${INSTALL_FILE_}';\n"
              "files info: '${CPACK_DEB_INSTALL_FILES}';\n"
              "binary files: '${CPACK_DEB_BINARY_FILES}'")
        endif()

        #Get rid of prefix generated by dpkg-shlibdeps
        string(REGEX REPLACE "^.*Depends=" "" CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS "${SHLIBDEPS_OUTPUT}")

        if(CPACK_DEBIAN_PACKAGE_DEBUG)
          message("CPackDeb Debug: Found dependency: ${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS} from output ${SHLIBDEPS_OUTPUT}")
        endif()

        # Remove blank control file
        # Might not be safe if package actual contain file or directory named debian
        file(REMOVE_RECURSE "${CPACK_TEMPORARY_DIRECTORY}/debian")
      else()
        if(CPACK_DEBIAN_PACKAGE_DEBUG)
          message(AUTHOR_WARNING "CPackDeb Debug: Using only user-provided depends because package does not contain executable files that link to shared libraries.")
        endif()
      endif()
    else()
      message("CPackDeb: Using only user-provided dependencies because dpkg-shlibdeps is not found.")
    endif()

  else()
    if(CPACK_DEBIAN_PACKAGE_DEBUG)
      message("CPackDeb Debug: Using only user-provided dependencies")
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

  # Source: (optional)
  # in case several packages are constructed from a unique source
  # (multipackaging), the source may be indicated as well.
  # The source might contain a version if the generated package
  # version is different from the source version
  if(NOT CPACK_DEBIAN_PACKAGE_SOURCE)
    set(CPACK_DEBIAN_PACKAGE_SOURCE "")
  endif()

  # have a look at get_property(result GLOBAL PROPERTY ENABLED_FEATURES),
  # this returns the successful find_package() calls, maybe this can help
  # Depends:
  # You should set: DEBIAN_PACKAGE_DEPENDS
  # TODO: automate 'objdump -p | grep NEEDED'

  # if per-component variable, overrides the global CPACK_DEBIAN_PACKAGE_${variable_type_}
  # automatic dependency discovery will be performed afterwards.
  if(CPACK_DEB_PACKAGE_COMPONENT)
    foreach(value_type_ DEPENDS RECOMMENDS SUGGESTS PREDEPENDS ENHANCES BREAKS CONFLICTS PROVIDES REPLACES SOURCE SECTION PRIORITY NAME)
      set(_component_var "CPACK_DEBIAN_${_local_component_name}_PACKAGE_${value_type_}")

      # if set, overrides the global variable
      if(DEFINED ${_component_var})
        set(CPACK_DEBIAN_PACKAGE_${value_type_} "${${_component_var}}")
        if(CPACK_DEBIAN_PACKAGE_DEBUG)
          message("CPackDeb Debug: component '${_local_component_name}' ${value_type_} "
            "value set to '${CPACK_DEBIAN_PACKAGE_${value_type_}}'")
        endif()
      endif()
    endforeach()
  endif()

  # at this point, the CPACK_DEBIAN_PACKAGE_DEPENDS is properly set
  # to the minimal dependency of the package
  # Append automatically discovered dependencies .
  if(NOT "${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS}" STREQUAL "")
    if (CPACK_DEBIAN_PACKAGE_DEPENDS)
      set (CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS}, ${CPACK_DEBIAN_PACKAGE_DEPENDS}")
    else ()
      set (CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_AUTO_DEPENDS}")
    endif ()
  endif()

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
  if(NOT CPACK_DEB_PACKAGE_COMPONENT)
    if(NOT CPACK_DEBIAN_PACKAGE_DESCRIPTION)
      if(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
        message(FATAL_ERROR "CPackDeb: Debian package requires a summary for a package, set CPACK_PACKAGE_DESCRIPTION_SUMMARY or CPACK_DEBIAN_PACKAGE_DESCRIPTION")
      endif()
      set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    endif()
  else()
    set(component_description_var CPACK_COMPONENT_${_local_component_name}_DESCRIPTION)

    # component description overrides package description
    if(${component_description_var})
      set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${${component_description_var}})
    elseif(NOT CPACK_DEBIAN_PACKAGE_DESCRIPTION)
      if(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY)
        message(FATAL_ERROR "CPackDeb: Debian package requires a summary for a package, set CPACK_PACKAGE_DESCRIPTION_SUMMARY or CPACK_DEBIAN_PACKAGE_DESCRIPTION or ${component_description_var}")
      endif()
      set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    endif()
  endif()

  # Section: (recommended)
  if(NOT CPACK_DEBIAN_PACKAGE_SECTION)
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
  endif()

  # Priority: (recommended)
  if(NOT CPACK_DEBIAN_PACKAGE_PRIORITY)
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  endif()

  # Compression: (recommended)
  if(NOT CPACK_DEBIAN_COMPRESSION_TYPE)
    set(CPACK_DEBIAN_COMPRESSION_TYPE "gzip")
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
  # - prerm
  # Usage:
  # set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  #    "${CMAKE_CURRENT_SOURCE_DIR/prerm;${CMAKE_CURRENT_SOURCE_DIR}/postrm")

  # Are we packaging components ?
  if(CPACK_DEB_PACKAGE_COMPONENT)
    # override values with per component version if set
    foreach(VAR_NAME_ "PACKAGE_CONTROL_EXTRA" "PACKAGE_CONTROL_STRICT_PERMISSION")
      if(CPACK_DEBIAN_${_local_component_name}_${VAR_NAME_})
        set(CPACK_DEBIAN_${VAR_NAME_} "${CPACK_DEBIAN_${_local_component_name}_${VAR_NAME_}}")
      endif()
    endforeach()

    if(CPACK_DEBIAN_${_local_component_name}_PACKAGE_NAME)
      string(TOLOWER "${CPACK_DEBIAN_${_local_component_name}_PACKAGE_NAME}" CPACK_DEBIAN_PACKAGE_NAME)
    else()
      string(TOLOWER "${CPACK_DEBIAN_PACKAGE_NAME}-${CPACK_DEB_PACKAGE_COMPONENT}" CPACK_DEBIAN_PACKAGE_NAME)
    endif()
  endif()

  # Print out some debug information if we were asked for that
  if(CPACK_DEBIAN_PACKAGE_DEBUG)
     message("CPackDeb:Debug: CPACK_TOPLEVEL_DIRECTORY          = '${CPACK_TOPLEVEL_DIRECTORY}'")
     message("CPackDeb:Debug: CPACK_TOPLEVEL_TAG                = '${CPACK_TOPLEVEL_TAG}'")
     message("CPackDeb:Debug: CPACK_TEMPORARY_DIRECTORY         = '${CPACK_TEMPORARY_DIRECTORY}'")
     message("CPackDeb:Debug: CPACK_OUTPUT_FILE_NAME            = '${CPACK_OUTPUT_FILE_NAME}'")
     message("CPackDeb:Debug: CPACK_OUTPUT_FILE_PATH            = '${CPACK_OUTPUT_FILE_PATH}'")
     message("CPackDeb:Debug: CPACK_PACKAGE_FILE_NAME           = '${CPACK_PACKAGE_FILE_NAME}'")
     message("CPackDeb:Debug: CPACK_PACKAGE_INSTALL_DIRECTORY   = '${CPACK_PACKAGE_INSTALL_DIRECTORY}'")
     message("CPackDeb:Debug: CPACK_TEMPORARY_PACKAGE_FILE_NAME = '${CPACK_TEMPORARY_PACKAGE_FILE_NAME}'")
     message("CPackDeb:Debug: CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION = '${CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION}'")
     message("CPackDeb:Debug: CPACK_DEBIAN_PACKAGE_SOURCE       = '${CPACK_DEBIAN_PACKAGE_SOURCE}'")
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

  # move variables to parent scope so that they may be used to create debian package
  set(GEN_CPACK_DEBIAN_PACKAGE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_DEBIAN_PACKAGE_VERSION}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_SECTION "${CPACK_DEBIAN_PACKAGE_SECTION}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_PRIORITY "${CPACK_DEBIAN_PACKAGE_PRIORITY}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_DEBIAN_PACKAGE_MAINTAINER}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_COMPRESSION_TYPE "${CPACK_DEBIAN_COMPRESSION_TYPE}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_RECOMMENDS "${CPACK_DEBIAN_PACKAGE_RECOMMENDS}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_SUGGESTS "${CPACK_DEBIAN_PACKAGE_SUGGESTS}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_DEBIAN_PACKAGE_HOMEPAGE}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_PREDEPENDS "${CPACK_DEBIAN_PACKAGE_PREDEPENDS}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_ENHANCES "${CPACK_DEBIAN_PACKAGE_ENHANCES}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_BREAKS "${CPACK_DEBIAN_PACKAGE_BREAKS}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_CONFLICTS "${CPACK_DEBIAN_PACKAGE_CONFLICTS}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_PROVIDES "${CPACK_DEBIAN_PACKAGE_PROVIDES}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_REPLACES "${CPACK_DEBIAN_PACKAGE_REPLACES}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION
      "${CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION}" PARENT_SCOPE)
  set(GEN_CPACK_DEBIAN_PACKAGE_SOURCE
     "${CPACK_DEBIAN_PACKAGE_SOURCE}" PARENT_SCOPE)
  set(GEN_WDIR "${WDIR}" PARENT_SCOPE)
endfunction()

cpack_deb_prepare_package_vars()
