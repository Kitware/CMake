PROJECT(CMake)
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# The CMake version number.
SET(CMake_VERSION_MAJOR 2)
SET(CMake_VERSION_MINOR 4)
SET(CMake_VERSION_PATCH 8)
# for an actual release this should not be defined
#SET(CMake_VERSION_RC 13)
SET(CMake_VERSION "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CMake_VERSION_FULL "${CMake_VERSION}.${CMake_VERSION_PATCH}")

SET(BUILD_SHARED_LIBS OFF)

# The CMake executables do not need any rpath to run in the build or
# install tree.
SET(CMAKE_SKIP_RPATH ON CACHE INTERNAL "CMake does not need RPATHs.")

SET(CMAKE_DATA_DIR "/share/cmake-${CMake_VERSION}" CACHE STRING
  "Install location for data (relative to prefix).")
SET(CMAKE_DOC_DIR "/doc/cmake-${CMake_VERSION}" CACHE STRING
  "Install location for documentation (relative to prefix).")
SET(CMAKE_MAN_DIR "/man" CACHE STRING
  "Install location for man pages (relative to prefix).")
MARK_AS_ADVANCED(CMAKE_DATA_DIR CMAKE_DOC_DIR CMAKE_MAN_DIR)

IF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 1.4)
  IF(CMAKE_GENERATOR MATCHES "Visual Studio 7")
    SET(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
  ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 7")
  IF(CMAKE_GENERATOR MATCHES "Visual Studio 6")
    SET(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
  ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio 6")
  INCLUDE (${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityCXX.cmake)
ENDIF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 1.4)

# Disable deprecation warnings for standard C functions.
IF(CMAKE_COMPILER_2005)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF(CMAKE_COMPILER_2005)

#silence duplicate symbol warnings on AIX
IF(CMAKE_SYSTEM MATCHES "AIX.*")
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -bhalt:5 ")
  ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM MATCHES "AIX.*")

IF(CMAKE_SYSTEM MATCHES "IRIX.*")
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-woff84 -no_auto_include")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-woff15")
  ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM MATCHES "IRIX.*")

IF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local -no_implicit_include ")
  ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM MATCHES "OSF1-V.*")

#-----------------------------------------------------------------------------
# Create the kwsys library for CMake.
SET(KWSYS_NAMESPACE cmsys)
SET(KWSYS_USE_SystemTools 1)
SET(KWSYS_USE_Directory 1)
SET(KWSYS_USE_RegularExpression 1)
SET(KWSYS_USE_Base64 1)
SET(KWSYS_USE_Process 1)
SET(KWSYS_USE_CommandLineArguments 1)
SET(KWSYS_HEADER_ROOT ${CMake_BINARY_DIR}/Source)
SUBDIRS(Source/kwsys)

#-----------------------------------------------------------------------------
# Setup third-party libraries.

# Everything in the tree should be able to include files from the
# Utilities directory.
INCLUDE_DIRECTORIES(
  ${CMake_SOURCE_DIR}/Utilities
  ${CMake_BINARY_DIR}/Utilities
  )

# Third party libraries must be something that can be found.
IF(EXISTS ${CMAKE_ROOT}/Modules/FindXMLRPC.cmake)
  SET(CMAKE_ALLOW_SYSTEM_LIBRARIES 1)
ELSE(EXISTS ${CMAKE_ROOT}/Modules/FindXMLRPC.cmake)
  SET(CMAKE_ALLOW_SYSTEM_LIBRARIES 0)
ENDIF(EXISTS ${CMAKE_ROOT}/Modules/FindXMLRPC.cmake)

IF(CMAKE_ALLOW_SYSTEM_LIBRARIES)
  # Options have dependencies.
  INCLUDE(CMakeDependentOption)

  # Allow the user to enable/disable all system utility library options
  # by setting CMAKE_USE_SYSTEM_LIBRARIES on the command line.
  IF(DEFINED CMAKE_USE_SYSTEM_LIBRARIES)
    SET(CMAKE_USE_SYSTEM_LIBRARIES_USER 1)
  ENDIF(DEFINED CMAKE_USE_SYSTEM_LIBRARIES)
  IF(CMAKE_USE_SYSTEM_LIBRARIES)
    SET(CMAKE_USE_SYSTEM_LIBRARIES ON)
  ELSE(CMAKE_USE_SYSTEM_LIBRARIES)
    SET(CMAKE_USE_SYSTEM_LIBRARIES OFF)
  ENDIF(CMAKE_USE_SYSTEM_LIBRARIES)
  IF(CMAKE_USE_SYSTEM_LIBRARIES_USER)
    SET(CMAKE_USE_SYSTEM_CURL "${CMAKE_USE_SYSTEM_LIBRARIES}" CACHE BOOL "Use system-installed curl" FORCE)
    SET(CMAKE_USE_SYSTEM_EXPAT "${CMAKE_USE_SYSTEM_LIBRARIES}" CACHE BOOL "Use system-installed expat" FORCE)
    SET(CMAKE_USE_SYSTEM_XMLRPC "${CMAKE_USE_SYSTEM_LIBRARIES}" CACHE BOOL "Use system-installed xmlrpc" FORCE)
    SET(CMAKE_USE_SYSTEM_ZLIB "${CMAKE_USE_SYSTEM_LIBRARIES}" CACHE BOOL "Use system-installed zlib" FORCE)
  ENDIF(CMAKE_USE_SYSTEM_LIBRARIES_USER)

  # Optionally use system utility libraries.
  OPTION(CMAKE_USE_SYSTEM_CURL   "Use system-installed curl" ${CMAKE_USE_SYSTEM_LIBRARIES})
  OPTION(CMAKE_USE_SYSTEM_XMLRPC "Use system-installed xmlrpc" ${CMAKE_USE_SYSTEM_LIBRARIES})
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_EXPAT "Use system-installed expat"
    ${CMAKE_USE_SYSTEM_LIBRARIES} "NOT CMAKE_USE_SYSTEM_XMLRPC" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_ZLIB "Use system-installed zlib"
    ${CMAKE_USE_SYSTEM_LIBRARIES} "NOT CMAKE_USE_SYSTEM_CURL" ON)

  # There is currently no option for system tar because the upstream
  # libtar does not have our modifications to allow reentrant
  # object-oriented use of the library.
  # OPTION(CMAKE_USE_SYSTEM_TAR    "Use system-installed tar"   OFF)
ELSE(CMAKE_ALLOW_SYSTEM_LIBRARIES)
  SET(CMAKE_USE_SYSTEM_CURL 0)
  SET(CMAKE_USE_SYSTEM_EXPAT 0)
  SET(CMAKE_USE_SYSTEM_XMLRPC 0)
  SET(CMAKE_USE_SYSTEM_ZLIB 0)
ENDIF(CMAKE_ALLOW_SYSTEM_LIBRARIES)

# Inform utility library header wrappers whether to use system versions.
CONFIGURE_FILE(${CMake_SOURCE_DIR}/Utilities/cmThirdParty.h.in
               ${CMake_BINARY_DIR}/Utilities/cmThirdParty.h
               @ONLY IMMEDIATE)

# Mention to the user what system libraries are being used.
FOREACH(util CURL EXPAT XMLRPC ZLIB)
  IF(CMAKE_USE_SYSTEM_${util})
    MESSAGE(STATUS "Using system-installed ${util}")
  ENDIF(CMAKE_USE_SYSTEM_${util})
ENDFOREACH(util)

#-----------------------------------------------------------------------------
# Build zlib library for Curl, CMake, and CTest.
SET(CMAKE_ZLIB_HEADER "cm_zlib.h")
IF(CMAKE_USE_SYSTEM_ZLIB)
  FIND_PACKAGE(ZLIB)
  IF(NOT ZLIB_FOUND)
    MESSAGE(FATAL_ERROR "CMAKE_USE_SYSTEM_ZLIB is ON but a zlib is not found!")
  ENDIF(NOT ZLIB_FOUND)
  SET(CMAKE_ZLIB_INCLUDES ${ZLIB_INCLUDE_DIR})
  SET(CMAKE_ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
ELSE(CMAKE_USE_SYSTEM_ZLIB)
  SUBDIRS(Utilities/cmzlib)
  SET(CMAKE_ZLIB_INCLUDES)
  SET(CMAKE_ZLIB_LIBRARIES cmzlib)
ENDIF(CMAKE_USE_SYSTEM_ZLIB)

#-----------------------------------------------------------------------------
# Build Curl library for CTest.
IF(CMAKE_USE_SYSTEM_CURL)
  FIND_PACKAGE(CURL)
  IF(NOT CURL_FOUND)
    MESSAGE(FATAL_ERROR "CMAKE_USE_SYSTEM_CURL is ON but a curl is not found!")
  ENDIF(NOT CURL_FOUND)
  SET(CMAKE_CURL_INCLUDES ${CURL_INCLUDE_DIRS})
  SET(CMAKE_CURL_LIBRARIES ${CURL_LIBRARIES})
ELSE(CMAKE_USE_SYSTEM_CURL)
  SET(CURL_SPECIAL_ZLIB_H ${CMAKE_ZLIB_HEADER})
  SET(CURL_SPECIAL_LIBZ_INCLUDES ${CMAKE_ZLIB_INCLUDES})
  SET(CURL_SPECIAL_LIBZ ${CMAKE_ZLIB_LIBRARIES})
  SUBDIRS(Utilities/cmcurl)
  SET(CMAKE_CURL_INCLUDES)
  SET(CMAKE_CURL_LIBRARIES cmcurl)
ENDIF(CMAKE_USE_SYSTEM_CURL)

#-----------------------------------------------------------------------------
# Build Tar library for CTest.
SET(CMTAR_ZLIB_HEADER ${CMAKE_ZLIB_HEADER})
SET(CMTAR_ZLIB_LIBRARIES ${CMAKE_ZLIB_LIBRARIES})
SET(CMTAR_ZLIB_INCLUDE_DIRS ${CMAKE_ZLIB_INCLUDES})
SUBDIRS(Utilities/cmtar)
SET(CMAKE_TAR_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmtar)
SET(CMAKE_TAR_LIBRARIES cmtar)

#-----------------------------------------------------------------------------
# Build Compress library for CTest.
SUBDIRS(Utilities/cmcompress)
SET(CMAKE_COMPRESS_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmcompress")
SET(CMAKE_COMPRESS_LIBRARIES "cmcompress")

#-----------------------------------------------------------------------------
# Build expat library for CMake and CTest.
IF(CMAKE_USE_SYSTEM_EXPAT)
  FIND_PACKAGE(EXPAT)
  IF(NOT EXPAT_FOUND)
    MESSAGE(FATAL_ERROR
      "CMAKE_USE_SYSTEM_EXPAT is ON but a expat is not found!")
  ENDIF(NOT EXPAT_FOUND)
  SET(CMAKE_EXPAT_INCLUDES ${EXPAT_INCLUDE_DIRS})
  SET(CMAKE_EXPAT_LIBRARIES ${EXPAT_LIBRARIES})
ELSE(CMAKE_USE_SYSTEM_EXPAT)
  SUBDIRS(Utilities/cmexpat)
  SET(CMAKE_EXPAT_INCLUDES)
  SET(CMAKE_EXPAT_LIBRARIES cmexpat)
ENDIF(CMAKE_USE_SYSTEM_EXPAT)

IF(CMAKE_USE_SYSTEM_XMLRPC)
  FIND_PACKAGE(XMLRPC QUIET REQUIRED libwww-client)
  IF(NOT XMLRPC_FOUND)
    MESSAGE(FATAL_ERROR
      "CMAKE_USE_SYSTEM_XMLRPC is ON but a xmlrpc is not found!")
  ENDIF(NOT XMLRPC_FOUND)
  SET(CMAKE_XMLRPC_INCLUDES ${XMLRPC_INCLUDE_DIRS})
  SET(CMAKE_XMLRPC_LIBRARIES ${XMLRPC_LIBRARIES})
ELSE(CMAKE_USE_SYSTEM_XMLRPC)
  SUBDIRS(Utilities/cmxmlrpc)
  SET(CMAKE_XMLRPC_INCLUDES)
  SET(CMAKE_XMLRPC_LIBRARIES cmXMLRPC)
ENDIF(CMAKE_USE_SYSTEM_XMLRPC)

IF (UNIX)
  FIND_PACKAGE(Curses QUIET)
  IF (CURSES_LIBRARY)
    OPTION(BUILD_CursesDialog "Build the CMake Curses Dialog ccmake" ON)
  ELSE (CURSES_LIBRARY)
    MESSAGE("Curses libraries were not found. Curses GUI for CMake will not be build.")
    SET(BUILD_CursesDialog 0)
  ENDIF (CURSES_LIBRARY)
ELSE (UNIX)
  SET(BUILD_CursesDialog 0)
ENDIF (UNIX)

IF(BUILD_CursesDialog)
  SUBDIRS(Source/CursesDialog/form)
ENDIF(BUILD_CursesDialog)

INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(unsetenv "stdlib.h" HAVE_UNSETENV)
CHECK_SYMBOL_EXISTS(environ "stdlib.h" HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE)

SUBDIRS(Source Modules Templates Utilities Tests/CMakeTests)
ENABLE_TESTING()

# Include the standard Dart testing module
#SET(DART_ROOT "" CACHE INTERNAL "" FORCE)
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

# use the ansi CXX compile flag for building cmake

IF (CMAKE_ANSI_CXXFLAGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
ENDIF (CMAKE_ANSI_CXXFLAGS)

IF (CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")  
ENDIF (CMAKE_ANSI_CFLAGS)

# add some testing dependencies
IF(BUILD_TESTING)
  IF (DART_ROOT)
    CONFIGURE_FILE(${CMake_SOURCE_DIR}/CMakeLogo.gif ${CMake_BINARY_DIR}/Testing/HTML/TestingResults/Icons/Logo.gif COPYONLY)
  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)
MARK_AS_ADVANCED(DART_ROOT)
MARK_AS_ADVANCED(CURL_TESTING)

SET(EXECUTABLE_OUTPUT_PATH ${CMake_BINARY_DIR}/bin CACHE INTERNAL 
  "Where to put the executables for CMake"
  )
SET(LIBRARY_OUTPUT_PATH "" CACHE INTERNAL 
  "Where to put the libraries for CMake"
  )

INCLUDE_REGULAR_EXPRESSION("^.*$")

CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/.NoDartCoverage
  ${CMake_BINARY_DIR}/Tests/.NoDartCoverage)
CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/.NoDartCoverage
  ${CMake_BINARY_DIR}/Modules/.NoDartCoverage)

CONFIGURE_FILE(${CMake_SOURCE_DIR}/CTestCustom.ctest.in
  ${CMake_BINARY_DIR}/CTestCustom.ctest @ONLY)

SET(CMAKE_BUILD_ON_VISUAL_STUDIO 0)
IF(WIN32)
  IF(NOT UNIX)
    IF(NOT BORLAND)
      IF(NOT MINGW )
        SET(CMAKE_BUILD_ON_VISUAL_STUDIO 1)
      ENDIF(NOT MINGW )
    ENDIF(NOT BORLAND)
  ENDIF(NOT UNIX)
ENDIF(WIN32)
IF(CMAKE_BUILD_ON_VISUAL_STUDIO)
  IF("CMake_HAVE_MFC" MATCHES "^CMake_HAVE_MFC$")
    SET(CHECK_INCLUDE_FILE_VAR "afxwin.h")
    CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.cxx IMMEDIATE)
    MESSAGE(STATUS "Looking for MFC")
    TRY_COMPILE(CMake_HAVE_MFC
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.cxx
      CMAKE_FLAGS
      -DCMAKE_MFC_FLAG:STRING=2
      -DCOMPILE_DEFINITIONS:STRING=-D_AFXDLL
      OUTPUT_VARIABLE OUTPUT)
    IF(CMake_HAVE_MFC)
      MESSAGE(STATUS "Looking for MFC - found")
      SET(CMake_HAVE_MFC 1 CACHE INTERNAL "Have MFC?")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if MFC exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(CMake_HAVE_MFC)
      MESSAGE(STATUS "Looking for MFC - not found")
      SET(CMake_HAVE_MFC 0 CACHE INTERNAL "Have MFC?")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if MFC exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(CMake_HAVE_MFC)
  ENDIF("CMake_HAVE_MFC" MATCHES "^CMake_HAVE_MFC$")

  IF(CMake_HAVE_MFC)
    OPTION(BUILD_MFCDialog "Whether to build the CMakeSetup MFC dialog." ON)
  ELSE(CMake_HAVE_MFC)
    SET(BUILD_MFCDialog 0)
  ENDIF(CMake_HAVE_MFC)
ELSE(CMAKE_BUILD_ON_VISUAL_STUDIO)
  SET(BUILD_MFCDialog 0)
ENDIF(CMAKE_BUILD_ON_VISUAL_STUDIO)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# include support for making the release
INCLUDE (${CMake_SOURCE_DIR}/Utilities/Release/Release.cmake)

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
  SET(CPACK_PACKAGE_VERSION_PATCH "${CMake_VERSION_PATCH}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME
    "cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}")
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
  IF(WIN32 AND NOT UNIX)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\CMakeInstall.bmp")
    SET(CPACK_PACKAGE_EXECUTABLES "CMakeSetup" "CMake")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\CMakeSetup.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} a cross-platform, open-source build system")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.cmake.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.kitware.com")
    SET(CPACK_NSIS_CONTACT "cmake@cmake.org")
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
      "${CPACK_PACKAGE_NAME}-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}")
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
    SUBDIRS(Utilities/Release/Cygwin)
    # when packaging source make sure the .build directory is not included
    SET(CPACK_SOURCE_IGNORE_FILES
      "/CVS/" "/\\\\.build/" "/\\\\.svn/" "\\\\.swp$" "\\\\.#" "/#" "~$")
  ENDIF(CYGWIN)
  # include CPack model once all variables are set
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

# configure the CTestScript.cmake file into the binary directory
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/Templates/CTestScript.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CTestScript.cmake"
  @ONLY IMMEDIATE)

# Testing
IF (NOT DART_ROOT)
  SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
ENDIF (NOT DART_ROOT)

IF(BUILD_TESTING)
  SET(CMAKE_TEST_GENERATOR "" CACHE STRING "Generator used when running tests")
  SET(CMAKE_TEST_MAKEPROGRAM "" CACHE FILEPATH "Generator used when running tests")
  IF(NOT CMAKE_TEST_GENERATOR)
    SET(CMAKE_TEST_GENERATOR "${CMAKE_GENERATOR}")
    SET(CMAKE_TEST_MAKEPROGRAM "${MAKEPROGRAM}")
  ELSE(NOT CMAKE_TEST_GENERATOR)
    SET(CMAKE_TEST_DIFFERENT_GENERATOR TRUE)
  ENDIF(NOT CMAKE_TEST_GENERATOR)

  # Are we testing with the MSVC compiler?
  SET(CMAKE_TEST_MSVC 0)
  IF(MSVC AND NOT CMAKE_TEST_DIFFERENT_GENERATOR)
    SET(CMAKE_TEST_MSVC 1)
  ELSE(MSVC AND NOT CMAKE_TEST_DIFFERENT_GENERATOR)
    IF("${CMAKE_TEST_GENERATOR}" MATCHES "NMake" OR
        "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio")
      SET(CMAKE_TEST_MSVC 1)
    ENDIF("${CMAKE_TEST_GENERATOR}" MATCHES "NMake" OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio")
  ENDIF(MSVC AND NOT CMAKE_TEST_DIFFERENT_GENERATOR)

  SET(CMAKE_TEST_SYSTEM_LIBRARIES 0)
  FOREACH(util CURL EXPAT XMLRPC ZLIB)
    IF(CMAKE_USE_SYSTEM_${util})
      SET(CMAKE_TEST_SYSTEM_LIBRARIES 1)
    ENDIF(CMAKE_USE_SYSTEM_${util})
  ENDFOREACH(util)

  # This variable is set by cmake, however to
  # test cmake we want to make sure that
  # the ctest from this cmake is used for testing
  # and not the ctest from the cmake building and testing
  # cmake.
  SET(CMAKE_CTEST_COMMAND "${EXECUTABLE_OUTPUT_PATH}/ctest")
  SET(CMAKE_CMAKE_COMMAND "${EXECUTABLE_OUTPUT_PATH}/cmake")
ENDIF(BUILD_TESTING)
