# If the cmake version includes cpack, use it
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  IF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")
    SET(CMAKE_INSTALL_MFC_LIBRARIES 1)
    OPTION(CMAKE_INSTALL_DEBUG_LIBRARIES 
      "Install Microsoft runtime debug libraries with CMake." FALSE)
    MARK_AS_ADVANCED(CMAKE_INSTALL_DEBUG_LIBRARIES)
    INCLUDE(${CMake_SOURCE_DIR}/Modules/InstallRequiredSystemLibraries.cmake)
  ENDIF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")
  # Set the options file that needs to be included inside CMakeCPackOptions.cmake
  SET(QT_DIALOG_CPACK_OPTIONS_FILE ${CMake_BINARY_DIR}/Source/QtDialog/QtDialogCPack.cmake)
  CONFIGURE_FILE("${CMake_SOURCE_DIR}/CMakeCPackOptions.cmake.in"
    "${CMake_BINARY_DIR}/CMakeCPackOptions.cmake" @ONLY)
  SET(CPACK_PROJECT_CONFIG_FILE "${CMake_BINARY_DIR}/CMakeCPackOptions.cmake")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMake is a build tool")
  SET(CPACK_PACKAGE_VENDOR "Kitware")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
  SET(CPACK_PACKAGE_VERSION_MAJOR "${CMake_VERSION_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${CMake_VERSION_MINOR}")
# if version date is set then use that as the patch 
  IF(CMake_VERSION_DATE)
    SET(CPACK_PACKAGE_VERSION_PATCH "${CMake_VERSION_DATE}")
  ELSE(CMake_VERSION_DATE)
    SET(CPACK_PACKAGE_VERSION_PATCH "${CMake_VERSION_PATCH}")
  ENDIF(CMake_VERSION_DATE)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME
    "cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
  IF(CMake_VERSION_RC)
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME
      "${CPACK_SOURCE_PACKAGE_FILE_NAME}-RC-${CMake_VERSION_RC}")
  ENDIF(CMake_VERSION_RC)
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
      SET(CPACK_SYSTEM_NAME win64-${CMAKE_SYSTEM_PROCESSOR})
    ELSE(CMAKE_CL_64)
      SET(CPACK_SYSTEM_NAME win32-${CMAKE_SYSTEM_PROCESSOR})
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
    
    # if we are on cygwin and have cpack, then force the 
    # doc, data and man dirs to conform to cygwin style directories
    SET(CMAKE_DOC_DIR "/share/doc/${CPACK_PACKAGE_FILE_NAME}")
    SET(CMAKE_DATA_DIR "/share/${CPACK_PACKAGE_FILE_NAME}")
    SET(CMAKE_MAN_DIR "/share/man")
    # let the user know we just forced these values
    MESSAGE(STATUS "Setup for Cygwin packaging")
    MESSAGE(STATUS "Override cache CMAKE_DOC_DIR = ${CMAKE_DOC_DIR}")
    MESSAGE(STATUS "Override cache CMAKE_DATA_DIR = ${CMAKE_DATA_DIR}")
    MESSAGE(STATUS "Override cache CMAKE_MAN_DIR = ${CMAKE_MAN_DIR}")
    
    # setup the cygwin package name
    SET(CPACK_PACKAGE_NAME cmake)
    # setup the name of the package for cygwin cmake-2.4.3
    SET(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME}-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
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
  # include CPack model once all variables are set
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
