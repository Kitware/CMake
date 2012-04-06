##section Variables common to all CPack generators
##end
##module
# - Build binary and source package installers.
# The CPack module generates binary and source installers in a variety
# of formats using the cpack program. Inclusion of the CPack module
# adds two new targets to the resulting makefiles, package and
# package_source, which build the binary and source installers,
# respectively. The generated binary installers contain everything
# installed via CMake's INSTALL command (and the deprecated
# INSTALL_FILES, INSTALL_PROGRAMS, and INSTALL_TARGETS commands).
#
# For certain kinds of binary installers (including the graphical
# installers on Mac OS X and Windows), CPack generates installers that
# allow users to select individual application components to
# install. See CPackComponent module for that.
#
# The CPACK_GENERATOR variable has different meanings in different
# contexts. In your CMakeLists.txt file, CPACK_GENERATOR is a
# *list of generators*: when run with no other arguments, CPack
# will iterate over that list and produce one package for each
# generator. In a CPACK_PROJECT_CONFIG_FILE, though, CPACK_GENERATOR
# is a *string naming a single generator*. If you need per-cpack-
# generator logic to control *other* cpack settings, then you need
# a CPACK_PROJECT_CONFIG_FILE.
#
# The CMake source tree itself contains a CPACK_PROJECT_CONFIG_FILE.
# See the top level file CMakeCPackOptions.cmake.in for an example.
#
# If set, the CPACK_PROJECT_CONFIG_FILE is included automatically
# on a per-generator basis. It only need contain overrides.
#
# Here's how it works:
#  - cpack runs
#  - it includes CPackConfig.cmake
#  - it iterates over the generators listed in that file's
#    CPACK_GENERATOR list variable (unless told to use just a
#    specific one via -G on the command line...)
#
#  - foreach generator, it then
#    - sets CPACK_GENERATOR to the one currently being iterated
#    - includes the CPACK_PROJECT_CONFIG_FILE
#    - produces the package for that generator
#
# This is the key: For each generator listed in CPACK_GENERATOR
# in CPackConfig.cmake, cpack will *reset* CPACK_GENERATOR
# internally to *the one currently being used* and then include
# the CPACK_PROJECT_CONFIG_FILE.
#
# Before including this CPack module in your CMakeLists.txt file,
# there are a variety of variables that can be set to customize
# the resulting installers. The most commonly-used variables are:
##end
#
##variable
#  CPACK_PACKAGE_NAME - The name of the package (or application). If
#  not specified, defaults to the project name.
##end
#
##variable
#  CPACK_PACKAGE_VENDOR - The name of the package vendor. (e.g.,
#  "Kitware").
##end
#
##variable
#  CPACK_PACKAGE_VERSION_MAJOR - Package major Version
##end
#
##variable
#  CPACK_PACKAGE_VERSION_MINOR - Package minor Version
##end
#
##variable
#  CPACK_PACKAGE_VERSION_PATCH - Package patch Version
##end
#
##variable
#  CPACK_PACKAGE_DESCRIPTION_FILE - A text file used to describe the
#  project. Used, for example, the introduction screen of a
#  CPack-generated Windows installer to describe the project.
##end
#
##variable
#  CPACK_PACKAGE_DESCRIPTION_SUMMARY - Short description of the
#  project (only a few words).
##end
#
##variable
#  CPACK_PACKAGE_FILE_NAME - The name of the package file to generate,
#  not including the extension. For example, cmake-2.6.1-Linux-i686.
#  The default value is
#  ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.
##end
#
##variable
#  CPACK_PACKAGE_INSTALL_DIRECTORY - Installation directory on the
#  target system. This may be used by some CPack generators
#  like NSIS to create an installation directory e.g., "CMake 2.5"
#  below the installation prefix. All installed element will be
#  put inside this directory.
##end
#
##variable
#   CPACK_PACKAGE_ICON - A branding image that will be displayed inside
#   the installer (used by GUI installers).
##end
#
##variable
#  CPACK_PROJECT_CONFIG_FILE - CPack-time project CPack configuration
#  file. This file included at cpack time, once per
#  generator after CPack has set CPACK_GENERATOR to the actual generator
#  being used. It allows per-generator setting of CPACK_* variables at
#  cpack time.
##end
#
##variable
#  CPACK_RESOURCE_FILE_LICENSE - License to be embedded in the installer. It
#  will typically be displayed to the user by the produced installer
#  (often with an explicit "Accept" button, for graphical installers)
#  prior to installation. This license file is NOT added to installed
#  file but is used by some CPack generators like NSIS. If you want
#  to install a license file (may be the same as this one)
#  along with your project you must add an appropriate CMake INSTALL
#  command in your CMakeLists.txt.
##end
#
##variable
#  CPACK_RESOURCE_FILE_README - ReadMe file to be embedded in the installer. It
#  typically describes in some detail the purpose of the project
#  during the installation. Not all CPack generators uses
#  this file.
##end
#
##variable
#  CPACK_RESOURCE_FILE_WELCOME - Welcome file to be embedded in the
#  installer. It welcomes users to this installer.
#  Typically used in the graphical installers on Windows and Mac OS X.
##end
#
##variable
#  CPACK_MONOLITHIC_INSTALL - Disables the component-based
#  installation mechanism. When set the component specification is ignored
#  and all installed items are put in a single "MONOLITHIC" package.
#  Some CPack generators do monolithic packaging by default and
#  may be asked to do component packaging by setting
#  CPACK_<GENNAME>_COMPONENT_INSTALL to 1/TRUE.
##end
#
##variable
#  CPACK_GENERATOR - List of CPack generators to use. If not
#  specified, CPack will create a set of options CPACK_BINARY_<GENNAME> (e.g.,
#  CPACK_BINARY_NSIS) allowing the user to enable/disable individual
#  generators. This variable may be used on the command line
#  as well as in:
#
#    cpack -D CPACK_GENERATOR="ZIP;TGZ" /path/to/build/tree
##end
#
##variable
#  CPACK_OUTPUT_CONFIG_FILE - The name of the CPack binary configuration
#  file. This file is the CPack configuration generated by the CPack module
#  for binary installers. Defaults to CPackConfig.cmake.
##end
#
##variable
#  CPACK_PACKAGE_EXECUTABLES - Lists each of the executables and associated
#  text label to be used to create Start Menu shortcuts. For example,
#  setting this to the list ccmake;CMake will
#  create a shortcut named "CMake" that will execute the installed
#  executable ccmake. Not all CPack generators use it (at least NSIS and
#  OSXX11 do).
##end
#
##variable
#  CPACK_STRIP_FILES - List of files to be stripped. Starting with
#  CMake 2.6.0 CPACK_STRIP_FILES will be a boolean variable which
#  enables stripping of all files (a list of files evaluates to TRUE
#  in CMake, so this change is compatible).
##end
#
# The following CPack variables are specific to source packages, and 
# will not affect binary packages:
#
##variable
#  CPACK_SOURCE_PACKAGE_FILE_NAME - The name of the source package. For
#  example cmake-2.6.1.
##end
#
##variable
#  CPACK_SOURCE_STRIP_FILES - List of files in the source tree that
#  will be stripped. Starting with CMake 2.6.0
#  CPACK_SOURCE_STRIP_FILES will be a boolean variable which enables
#  stripping of all files (a list of files evaluates to TRUE in CMake,
#  so this change is compatible).
##end
#
##variable
#  CPACK_SOURCE_GENERATOR - List of generators used for the source
#  packages. As with CPACK_GENERATOR, if this is not specified then
#  CPack will create a set of options (e.g., CPACK_SOURCE_ZIP)
#  allowing users to select which packages will be generated.
##end
#
##variable
#  CPACK_SOURCE_OUTPUT_CONFIG_FILE - The name of the CPack source
#  configuration file. This file is the CPack configuration generated by the
#  CPack module for source installers. Defaults to CPackSourceConfig.cmake.
##end
#
##variable
#  CPACK_SOURCE_IGNORE_FILES - Pattern of files in the source tree
#  that won't be packaged when building a source package. This is a
#  list of regular expression patterns (that must be properly escaped),
#  e.g., /CVS/;/\\.svn/;\\.swp$;\\.#;/#;.*~;cscope.*
##end
#
# The following variables are for advanced uses of CPack:
#
##variable
#  CPACK_CMAKE_GENERATOR - What CMake generator should be used if the
#  project is CMake project. Defaults to the value of CMAKE_GENERATOR
#  few users will want to change this setting.
##end
#
##variable
#  CPACK_INSTALL_CMAKE_PROJECTS - List of four values that specify
#  what project to install. The four values are: Build directory,
#  Project Name, Project Component, Directory. If omitted, CPack will
#  build an installer that installers everything.
##end
#
##variable
#  CPACK_SYSTEM_NAME - System name, defaults to the value of
#  ${CMAKE_SYSTEM_NAME}.
##end
#
##variable
#  CPACK_PACKAGE_VERSION - Package full version, used internally. By
#  default, this is built from CPACK_PACKAGE_VERSION_MAJOR,
#  CPACK_PACKAGE_VERSION_MINOR, and CPACK_PACKAGE_VERSION_PATCH.
##end
#
##variable
#  CPACK_TOPLEVEL_TAG - Directory for the installed files.
##end
#
##variable
#  CPACK_INSTALL_COMMANDS - Extra commands to install components.
##end
#
##variable
#  CPACK_INSTALLED_DIRECTORIES - Extra directories to install.
##end
#
##variable
#   CPACK_PACKAGE_INSTALL_REGISTRY_KEY - Registry key used when
#   installing this project. This is only used
#   by installer for Windows.
##end
##variable
#   CPACK_CREATE_DESKTOP_LINKS - List of desktop links to create.
##end
#

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

# Define this var in order to avoid (or warn) concerning multiple inclusion
IF(CPack_CMake_INCLUDED)
  MESSAGE(WARNING "CPack.cmake has already been included!!")
ELSE(CPack_CMake_INCLUDED)
  SET(CPack_CMake_INCLUDED 1)
ENDIF(CPack_CMake_INCLUDED)

# Pick a configuration file
SET(cpack_input_file "${CMAKE_ROOT}/Templates/CPackConfig.cmake.in")
IF(EXISTS "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
  SET(cpack_input_file "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
SET(cpack_source_input_file "${CMAKE_ROOT}/Templates/CPackConfig.cmake.in")
IF(EXISTS "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in")
  SET(cpack_source_input_file "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in")
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in")

# Backward compatibility
# Include CPackComponent macros if it has not already been included before.
include(CPackComponent)

# Macro for setting values if a user did not overwrite them
MACRO(cpack_set_if_not_set name value)
  IF(NOT DEFINED "${name}")
    SET(${name} "${value}")
  ENDIF(NOT DEFINED "${name}")
ENDMACRO(cpack_set_if_not_set)

# cpack_encode_variables - Macro to encode variables for the configuration file
# find any variable that starts with CPACK and create a variable
# _CPACK_OTHER_VARIABLES_ that contains SET commands for
# each cpack variable.  _CPACK_OTHER_VARIABLES_ is then
# used as an @ replacment in configure_file for the CPackConfig.
MACRO(cpack_encode_variables)
  SET(_CPACK_OTHER_VARIABLES_)
  GET_CMAKE_PROPERTY(res VARIABLES)
  FOREACH(var ${res})
    IF("xxx${var}" MATCHES "xxxCPACK")  
      SET(_CPACK_OTHER_VARIABLES_
        "${_CPACK_OTHER_VARIABLES_}\nSET(${var} \"${${var}}\")")
      ENDIF("xxx${var}" MATCHES "xxxCPACK")
  ENDFOREACH(var ${res})
ENDMACRO(cpack_encode_variables)

# Set the package name
cpack_set_if_not_set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_MAJOR "0")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_MINOR "1")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_PATCH "1")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
cpack_set_if_not_set(CPACK_PACKAGE_VENDOR "Humanity")
cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "${CMAKE_PROJECT_NAME} built using CMake")

cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_FILE
  "${CMAKE_ROOT}/Templates/CPack.GenericDescription.txt")
cpack_set_if_not_set(CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_ROOT}/Templates/CPack.GenericLicense.txt")
cpack_set_if_not_set(CPACK_RESOURCE_FILE_README
  "${CMAKE_ROOT}/Templates/CPack.GenericDescription.txt")
cpack_set_if_not_set(CPACK_RESOURCE_FILE_WELCOME
  "${CMAKE_ROOT}/Templates/CPack.GenericWelcome.txt")

cpack_set_if_not_set(CPACK_MODULE_PATH "${CMAKE_MODULE_PATH}")

IF(CPACK_NSIS_MODIFY_PATH)
  SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF(CPACK_NSIS_MODIFY_PATH)

SET(__cpack_system_name ${CMAKE_SYSTEM_NAME})
IF(${__cpack_system_name} MATCHES Windows)
  IF(CMAKE_CL_64)
    SET(__cpack_system_name win64)
  ELSE(CMAKE_CL_64)
    SET(__cpack_system_name win32)
  ENDIF(CMAKE_CL_64)
ENDIF(${__cpack_system_name} MATCHES Windows)
cpack_set_if_not_set(CPACK_SYSTEM_NAME "${__cpack_system_name}")

# Root dir: default value should be the string literal "$PROGRAMFILES"
# for backwards compatibility. Projects may set this value to anything.
set(__cpack_root_default "$PROGRAMFILES")
cpack_set_if_not_set(CPACK_NSIS_INSTALL_ROOT "${__cpack_root_default}")

# <project>-<major>.<minor>.<patch>-<release>-<platform>.<pkgtype>
cpack_set_if_not_set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
cpack_set_if_not_set(CPACK_PACKAGE_INSTALL_DIRECTORY
  "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
cpack_set_if_not_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY
  "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
cpack_set_if_not_set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
cpack_set_if_not_set(CPACK_PACKAGE_RELOCATABLE "true")

# always force to exactly "true" or "false" for CPack.Info.plist.in:
if(CPACK_PACKAGE_RELOCATABLE)
  set(CPACK_PACKAGE_RELOCATABLE "true")
else(CPACK_PACKAGE_RELOCATABLE)
  set(CPACK_PACKAGE_RELOCATABLE "false")
endif(CPACK_PACKAGE_RELOCATABLE)

macro(cpack_check_file_exists file description)
  if(NOT EXISTS "${file}")
    message(SEND_ERROR "CPack ${description} file: \"${file}\" could not be found.")
  endif(NOT EXISTS "${file}")
endmacro(cpack_check_file_exists)

cpack_check_file_exists("${CPACK_PACKAGE_DESCRIPTION_FILE}" "package description")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_LICENSE}"    "license resource")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_README}"     "readme resource")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_WELCOME}"    "welcome resource")

macro(cpack_optional_append _list _cond _item)
  if(${_cond})
    set(${_list} ${${_list}} ${_item})
  endif(${_cond})
endmacro(cpack_optional_append _list _cond _item)

##variable
# CPACK_BINARY_<GENNAME> - CPack generated options for binary generators. The
# CPack.cmake module generates (when CPACK_GENERATOR is not set)
# a set of CMake options (see CMake option command) which may then be used to
# select the CPack generator(s) to be used when launching the package target.
##end
# Provide options to choose generators
# we might check here if the required tools for the generates exist
# and set the defaults according to the results
if(NOT CPACK_GENERATOR)
  if(UNIX)
    if(CYGWIN)
      option(CPACK_BINARY_CYGWIN "Enable to build Cygwin binary packages" ON)
    else(CYGWIN)
      if(APPLE)
        option(CPACK_BINARY_BUNDLE       "Enable to build OSX bundles"      OFF)
        option(CPACK_BINARY_DRAGNDROP    "Enable to build OSX Drag And Drop package" OFF)
        option(CPACK_BINARY_PACKAGEMAKER "Enable to build PackageMaker packages" ON)
        option(CPACK_BINARY_OSXX11       "Enable to build OSX X11 packages"      OFF)
      else(APPLE)
        option(CPACK_BINARY_TZ  "Enable to build TZ packages"     ON)
      endif(APPLE)
      option(CPACK_BINARY_STGZ "Enable to build STGZ packages"    ON)
      option(CPACK_BINARY_TGZ  "Enable to build TGZ packages"     ON)
      option(CPACK_BINARY_TBZ2 "Enable to build TBZ2 packages"    OFF)
      option(CPACK_BINARY_DEB  "Enable to build Debian packages"  OFF)
      option(CPACK_BINARY_RPM  "Enable to build RPM packages"     OFF)
      option(CPACK_BINARY_NSIS "Enable to build NSIS packages"    OFF)
    endif(CYGWIN)
  else(UNIX)
    option(CPACK_BINARY_NSIS "Enable to build NSIS packages" ON)
    option(CPACK_BINARY_ZIP  "Enable to build ZIP packages" OFF)
  endif(UNIX)
  
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_BUNDLE       Bundle)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_DRAGNDROP    DragNDrop)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_PACKAGEMAKER PackageMaker)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_OSXX11       OSXX11)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_CYGWIN       CygwinBinary)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_DEB          DEB)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_RPM          RPM)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_NSIS         NSIS)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_STGZ         STGZ)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_TGZ          TGZ)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_TBZ2         TBZ2)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_TZ           TZ)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_ZIP          ZIP)
  
endif(NOT CPACK_GENERATOR)

# Provide options to choose source generators
if(NOT CPACK_SOURCE_GENERATOR)
  if(UNIX)
    if(CYGWIN)
      option(CPACK_SOURCE_CYGWIN "Enable to build Cygwin source packages" ON)
    else(CYGWIN)
      option(CPACK_SOURCE_TBZ2 "Enable to build TBZ2 source packages" ON)
      option(CPACK_SOURCE_TGZ  "Enable to build TGZ source packages"  ON)
      option(CPACK_SOURCE_TZ   "Enable to build TZ source packages"   ON)
      option(CPACK_SOURCE_ZIP  "Enable to build ZIP source packages"  OFF)
    endif(CYGWIN)
  else(UNIX)
    option(CPACK_SOURCE_ZIP "Enable to build ZIP source packages" ON)
  endif(UNIX)

  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_CYGWIN  CygwinSource)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_TGZ     TGZ)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_TBZ2    TBZ2)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_TZ      TZ)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_ZIP     ZIP)
endif(NOT CPACK_SOURCE_GENERATOR)

# mark the above options as advanced
mark_as_advanced(CPACK_BINARY_CYGWIN CPACK_BINARY_PACKAGEMAKER CPACK_BINARY_OSXX11
                 CPACK_BINARY_STGZ   CPACK_BINARY_TGZ          CPACK_BINARY_TBZ2 
                 CPACK_BINARY_DEB    CPACK_BINARY_RPM          CPACK_BINARY_TZ     
                 CPACK_BINARY_NSIS CPACK_BINARY_ZIP CPACK_BINARY_BUNDLE
                 CPACK_SOURCE_CYGWIN CPACK_SOURCE_TBZ2 CPACK_SOURCE_TGZ 
                 CPACK_SOURCE_TZ CPACK_SOURCE_ZIP CPACK_BINARY_DRAGNDROP)

# Set some other variables
cpack_set_if_not_set(CPACK_INSTALL_CMAKE_PROJECTS
  "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/")
cpack_set_if_not_set(CPACK_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
cpack_set_if_not_set(CPACK_TOPLEVEL_TAG "${CPACK_SYSTEM_NAME}")
# if the user has set CPACK_NSIS_DISPLAY_NAME remember it
if(DEFINED CPACK_NSIS_DISPLAY_NAME)
  SET(CPACK_NSIS_DISPLAY_NAME_SET TRUE)
endif()
# if the user has set CPACK_NSIS_DISPLAY
# explicitly, then use that as the default
# value of CPACK_NSIS_PACKAGE_NAME  instead
# of CPACK_PACKAGE_INSTALL_DIRECTORY 
cpack_set_if_not_set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")

if(CPACK_NSIS_DISPLAY_NAME_SET)
  string(REPLACE "\\" "\\\\" 
    _NSIS_DISPLAY_NAME_TMP  "${CPACK_NSIS_DISPLAY_NAME}")
  cpack_set_if_not_set(CPACK_NSIS_PACKAGE_NAME "${_NSIS_DISPLAY_NAME_TMP}")
else()
  cpack_set_if_not_set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
endif()

cpack_set_if_not_set(CPACK_OUTPUT_CONFIG_FILE
  "${CMAKE_BINARY_DIR}/CPackConfig.cmake")

cpack_set_if_not_set(CPACK_SOURCE_OUTPUT_CONFIG_FILE
  "${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake")

cpack_set_if_not_set(CPACK_SET_DESTDIR OFF)
cpack_set_if_not_set(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

cpack_set_if_not_set(CPACK_NSIS_INSTALLER_ICON_CODE "")
cpack_set_if_not_set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")

IF(DEFINED CPACK_COMPONENTS_ALL)
  IF(CPACK_MONOLITHIC_INSTALL)
    MESSAGE("CPack warning: both CPACK_COMPONENTS_ALL and CPACK_MONOLITHIC_INSTALL have been set.\nDefaulting to a monolithic installation.")
    SET(CPACK_COMPONENTS_ALL)
  ELSE(CPACK_MONOLITHIC_INSTALL)
    # The user has provided the set of components to be installed as
    # part of a component-based installation; trust her.
    SET(CPACK_COMPONENTS_ALL_SET_BY_USER TRUE)
  ENDIF(CPACK_MONOLITHIC_INSTALL)
ELSE(DEFINED CPACK_COMPONENTS_ALL)
  # If the user has not specifically requested a monolithic installer
  # but has specified components in various "install" commands, tell
  # CPack about those components.
  IF(NOT CPACK_MONOLITHIC_INSTALL)
    GET_CMAKE_PROPERTY(CPACK_COMPONENTS_ALL COMPONENTS)
    LIST(LENGTH CPACK_COMPONENTS_ALL CPACK_COMPONENTS_LEN)
    IF(CPACK_COMPONENTS_LEN EQUAL 1)
      # Only one component: this is not a component-based installation
      # (at least, it isn't a component-based installation, but may
      # become one later if the user uses the cpack_add_* commands).
      SET(CPACK_COMPONENTS_ALL)
    ENDIF(CPACK_COMPONENTS_LEN EQUAL 1)
    SET(CPACK_COMPONENTS_LEN)
  ENDIF(NOT CPACK_MONOLITHIC_INSTALL)
ENDIF(DEFINED CPACK_COMPONENTS_ALL)

# CMake always generates a component named "Unspecified", which is
# used to install everything that doesn't have an explicitly-provided
# component. Since these files should always be installed, we'll make
# them hidden and required.
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN TRUE)
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED TRUE)

cpack_encode_variables()
configure_file("${cpack_input_file}" "${CPACK_OUTPUT_CONFIG_FILE}" @ONLY IMMEDIATE)

# Generate source file
cpack_set_if_not_set(CPACK_SOURCE_INSTALLED_DIRECTORIES
  "${CMAKE_SOURCE_DIR};/")
cpack_set_if_not_set(CPACK_SOURCE_TOPLEVEL_TAG "${CPACK_SYSTEM_NAME}-Source")
cpack_set_if_not_set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source")
cpack_set_if_not_set(CPACK_SOURCE_IGNORE_FILES
  "/CVS/;/\\\\\\\\.svn/;/\\\\\\\\.bzr/;/\\\\\\\\.hg/;/\\\\\\\\.git/;\\\\\\\\.swp$;\\\\\\\\.#;/#")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_SOURCE_INSTALL_CMAKE_PROJECTS}")
SET(CPACK_INSTALLED_DIRECTORIES "${CPACK_SOURCE_INSTALLED_DIRECTORIES}")
SET(CPACK_GENERATOR "${CPACK_SOURCE_GENERATOR}")
SET(CPACK_TOPLEVEL_TAG "${CPACK_SOURCE_TOPLEVEL_TAG}")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_SOURCE_PACKAGE_FILE_NAME}")
SET(CPACK_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}")
SET(CPACK_STRIP_FILES "${CPACK_SOURCE_STRIP_FILES}")

cpack_encode_variables()
configure_file("${cpack_source_input_file}"
  "${CPACK_SOURCE_OUTPUT_CONFIG_FILE}" @ONLY IMMEDIATE)
