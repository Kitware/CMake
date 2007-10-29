# If the cmake version includes cpack, use it
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  IF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")
    SET(CMAKE_INSTALL_MFC_LIBRARIES 1)
    INCLUDE(InstallRequiredSystemLibraries)
  ENDIF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")
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
    SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
  ENDIF(NOT DEFINED CPACK_SYSTEM_NAME)
  IF(${CPACK_SYSTEM_NAME} MATCHES Windows)
    IF(CMAKE_CL_64)
      SET(CPACK_SYSTEM_NAME win64-${CMAKE_SYSTEM_PROCESSOR})
    ELSE(CMAKE_CL_64)
      SET(CPACK_SYSTEM_NAME win32-${CMAKE_SYSTEM_PROCESSOR})
    ENDIF(CMAKE_CL_64)
  ENDIF(${CPACK_SYSTEM_NAME} MATCHES Windows)
  IF(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
    SET(CPACK_PACKAGE_FILE_NAME "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${CPACK_SYSTEM_NAME}")
  ENDIF(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
  SET(CPACK_PACKAGE_CONTACT "cmake@cmake.org")
  IF(WIN32 AND NOT UNIX)
    # set the install/unistall icon used for the installer itself
    SET(CPACK_NSIS_MUI_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\CMakeLogo.ico")
    SET(CPACK_NSIS_MUI_UNIICON "${CMake_SOURCE_DIR}/Utilities/Release\\CMakeLogo.ico")
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\CMakeInstall.bmp")
    # tell cpack the executables you want in the start menu as links
    SET(CPACK_PACKAGE_EXECUTABLES "CMakeSetup" "CMake" )
    # tell cpack to create a desktop link to CMakeSetup
    SET(CPACK_CREATE_DESKTOP_LINK_CMakeSetup ON)
    # These variables should have escapes preserved during the 
    # translation to the CPackConfig.cmake file.  By default,
    # CPack will require double escapes as it gets parsed by
    # cmake twice
    SET(CPACK_ESCAPE_VARIABLES 
      CPACK_PACKAGE_ICON 
      CPACK_NSIS_MUI_ICON 
      CPACK_NSIS_MUI_UNIICON
      )
    # tell cpack to create links to the doc files
    SET(CPACK_NSIS_MENU_LINKS
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/CMakeSetup.html" "CMakeSetup Help"
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/cmake.html" "CMake Help"
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/cmake-properties.html"
      "CMake Properties and Variables Help"
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/ctest.html" "CTest Help"
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/cmake-modules.html" "CMake Modules Help"
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/cmake-commands.html" "CMake Commands Help"
      "doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}/cpack.html" "CPack Help"
      "http://www.cmake.org" "CMake Web Site"
)
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\CMakeSetup.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} a cross-platform, open-source build system")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.cmake.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.kitware.com")
    SET(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})
    SET(CPACK_NSIS_MODIFY_PATH ON)
  ELSE(WIN32 AND NOT UNIX)
    SET(CPACK_STRIP_FILES "bin/ccmake;bin/cmake;bin/cpack;bin/ctest")
    SET(CPACK_SOURCE_STRIP_FILES "")
    SET(CPACK_PACKAGE_EXECUTABLES "ccmake" "CMake")
  ENDIF(WIN32 AND NOT UNIX)
# cygwin specific packaging stuff
  IF(CYGWIN)
    SET(CPACK_PACKAGE_NAME cmake)
    # setup the name of the package for cygwin cmake-2.4.3
    SET(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME}-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
    # the source has the same name as the binary
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
    # Create a cygwin version number in case there are changes for cygwin
    # that are not reflected upstream in CMake
    SET(CPACK_CYGWIN_PATCH_NUMBER 1)
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
    # These files are required by the cmCPackCygwinSourceGenerator and the files
    # put into the release tar files.
    SET(CPACK_CYGWIN_BUILD_SCRIPT 
      "${CMake_BINARY_DIR}/@CPACK_PACKAGE_FILE_NAME@-@CPACK_CYGWIN_PATCH_NUMBER@.sh")
    SET(CPACK_CYGWIN_PATCH_FILE 
      "${CMake_BINARY_DIR}/@CPACK_PACKAGE_FILE_NAME@-@CPACK_CYGWIN_PATCH_NUMBER@.patch")
    # include the sub directory for cygwin releases
    INCLUDE(Utilities/Release/Cygwin)
    # when packaging source make sure the .build directory is not included
    SET(CPACK_SOURCE_IGNORE_FILES
      "/CVS/" "/\\\\.build/" "/\\\\.svn/" "\\\\.swp$" "\\\\.#" "/#" "~$")
  ENDIF(CYGWIN)
  # include CPack model once all variables are set
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
