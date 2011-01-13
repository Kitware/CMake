#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# If the cmake version includes cpack, use it
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  IF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")
    OPTION(CMAKE_INSTALL_DEBUG_LIBRARIES
      "Install Microsoft runtime debug libraries with CMake." FALSE)
    MARK_AS_ADVANCED(CMAKE_INSTALL_DEBUG_LIBRARIES)

    # By default, do not warn when built on machines using only VS Express:
    IF(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
      SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
    ENDIF()

    INCLUDE(${CMake_SOURCE_DIR}/Modules/InstallRequiredSystemLibraries.cmake)
  ENDIF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")

  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMake is a build tool")
  SET(CPACK_PACKAGE_VENDOR "Kitware")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
  SET(CPACK_PACKAGE_VERSION "${CMake_VERSION}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "cmake-${CMake_VERSION}")

  # Make this explicit here, rather than accepting the CPack default value,
  # so we can refer to it:
  SET(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")

  # Installers for 32- vs. 64-bit CMake:
  #  - Root install directory (displayed to end user at installer-run time)
  #  - "NSIS package/display name" (text used in the installer GUI)
  #  - Registry key used to store info about the installation
  IF(CMAKE_CL_64)
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    SET(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win64)")
    SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION} (Win64)")
  ELSE()
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    SET(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
  ENDIF()

  IF(NOT DEFINED CPACK_SYSTEM_NAME)
    # make sure package is not Cygwin-unknown, for Cygwin just
    # cygwin is good for the system name
    IF("${CMAKE_SYSTEM_NAME}" STREQUAL "CYGWIN")
      SET(CPACK_SYSTEM_NAME Cygwin)
    ELSE("${CMAKE_SYSTEM_NAME}" STREQUAL "CYGWIN")
      SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
    ENDIF("${CMAKE_SYSTEM_NAME}" STREQUAL "CYGWIN")
  ENDIF(NOT DEFINED CPACK_SYSTEM_NAME)
  IF(${CPACK_SYSTEM_NAME} MATCHES Windows)
    IF(CMAKE_CL_64)
      SET(CPACK_SYSTEM_NAME win64-x64)
    ELSE(CMAKE_CL_64)
      SET(CPACK_SYSTEM_NAME win32-x86)
    ENDIF(CMAKE_CL_64)
  ENDIF(${CPACK_SYSTEM_NAME} MATCHES Windows)

  IF(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
    # if the CPACK_PACKAGE_FILE_NAME is not defined by the cache
    # default to source package - system, on cygwin system is not 
    # needed
    IF(CYGWIN)
      SET(CPACK_PACKAGE_FILE_NAME "${CPACK_SOURCE_PACKAGE_FILE_NAME}")
    ELSE(CYGWIN)
      SET(CPACK_PACKAGE_FILE_NAME 
        "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${CPACK_SYSTEM_NAME}")
    ENDIF(CYGWIN)
  ENDIF(NOT DEFINED CPACK_PACKAGE_FILE_NAME)

  SET(CPACK_PACKAGE_CONTACT "cmake@cmake.org")

  IF(UNIX)
    SET(CPACK_STRIP_FILES "bin/ccmake;bin/cmake;bin/cpack;bin/ctest")
    SET(CPACK_SOURCE_STRIP_FILES "")
    SET(CPACK_PACKAGE_EXECUTABLES "ccmake" "CMake")
  ENDIF(UNIX)

  # cygwin specific packaging stuff
  IF(CYGWIN)
    # setup the cygwin package name
    SET(CPACK_PACKAGE_NAME cmake)
    # setup the name of the package for cygwin cmake-2.4.3
    SET(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME}-${CMake_VERSION}")
    # the source has the same name as the binary
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
    # Create a cygwin version number in case there are changes for cygwin
    # that are not reflected upstream in CMake
    SET(CPACK_CYGWIN_PATCH_NUMBER 1)
    # These files are required by the cmCPackCygwinSourceGenerator and the files
    # put into the release tar files.
    SET(CPACK_CYGWIN_BUILD_SCRIPT 
      "${CMake_BINARY_DIR}/@CPACK_PACKAGE_FILE_NAME@-@CPACK_CYGWIN_PATCH_NUMBER@.sh")
    SET(CPACK_CYGWIN_PATCH_FILE 
      "${CMake_BINARY_DIR}/@CPACK_PACKAGE_FILE_NAME@-@CPACK_CYGWIN_PATCH_NUMBER@.patch")
    # include the sub directory cmake file for cygwin that
    # configures some files and adds some install targets
    # this file uses some of the package file name variables
    INCLUDE(Utilities/Release/Cygwin/CMakeLists.txt)
  ENDIF(CYGWIN)

  # Set the options file that needs to be included inside CMakeCPackOptions.cmake
  SET(QT_DIALOG_CPACK_OPTIONS_FILE ${CMake_BINARY_DIR}/Source/QtDialog/QtDialogCPack.cmake)
  CONFIGURE_FILE("${CMake_SOURCE_DIR}/CMakeCPackOptions.cmake.in"
    "${CMake_BINARY_DIR}/CMakeCPackOptions.cmake" @ONLY)
  SET(CPACK_PROJECT_CONFIG_FILE "${CMake_BINARY_DIR}/CMakeCPackOptions.cmake")

  # include CPack model once all variables are set
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
