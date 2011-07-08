#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2011 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.3 FATAL_ERROR)
SET(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
PROJECT(CMake)

IF(CMAKE_BOOTSTRAP)
  # Running from bootstrap script.  Set local variable and remove from cache.
  SET(CMAKE_BOOTSTRAP 1)
  UNSET(CMAKE_BOOTSTRAP CACHE)
ENDIF()

SET(CMake_BIN_DIR ${CMake_BINARY_DIR}/bin)

IF("${CMake_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  # Disallow architecture-specific try_run.  It may not run on the host.
  MACRO(TRY_RUN)
    IF(CMAKE_TRY_COMPILE_OSX_ARCHITECTURES)
      MESSAGE(FATAL_ERROR "TRY_RUN not allowed with CMAKE_TRY_COMPILE_OSX_ARCHITECTURES=[${CMAKE_TRY_COMPILE_OSX_ARCHITECTURES}]")
    ELSE()
      _TRY_RUN(${ARGV})
    ENDIF()
  ENDMACRO()
ENDIF()

#-----------------------------------------------------------------------
# a macro to deal with system libraries, implemented as a macro
# simply to improve readability of the main script
#-----------------------------------------------------------------------
MACRO(CMAKE_HANDLE_SYSTEM_LIBRARIES)
  # Options have dependencies.
  INCLUDE(CMakeDependentOption)

  # Optionally use system xmlrpc.  We no longer build or use it by default.
  OPTION(CTEST_USE_XMLRPC "Enable xmlrpc submission method in CTest." OFF)
  MARK_AS_ADVANCED(CTEST_USE_XMLRPC)

  # Allow the user to enable/disable all system utility library options by
  # defining CMAKE_USE_SYSTEM_LIBRARIES or CMAKE_USE_SYSTEM_LIBRARY_${util}.
  SET(UTILITIES BZIP2 CURL EXPAT LIBARCHIVE ZLIB)
  FOREACH(util ${UTILITIES})
    IF(NOT DEFINED CMAKE_USE_SYSTEM_LIBRARY_${util}
        AND DEFINED CMAKE_USE_SYSTEM_LIBRARIES)
      SET(CMAKE_USE_SYSTEM_LIBRARY_${util} "${CMAKE_USE_SYSTEM_LIBRARIES}")
    ENDIF()
    IF(DEFINED CMAKE_USE_SYSTEM_LIBRARY_${util})
      IF(CMAKE_USE_SYSTEM_LIBRARY_${util})
        SET(CMAKE_USE_SYSTEM_LIBRARY_${util} ON)
      ELSE()
        SET(CMAKE_USE_SYSTEM_LIBRARY_${util} OFF)
      ENDIF()
      IF(CMAKE_BOOTSTRAP)
        UNSET(CMAKE_USE_SYSTEM_LIBRARY_${util} CACHE)
      ENDIF()
      STRING(TOLOWER "${util}" lutil)
      SET(CMAKE_USE_SYSTEM_${util} "${CMAKE_USE_SYSTEM_LIBRARY_${util}}"
        CACHE BOOL "Use system-installed ${lutil}" FORCE)
    ELSE()
      SET(CMAKE_USE_SYSTEM_LIBRARY_${util} OFF)
    ENDIF()
  ENDFOREACH(util)
  IF(CMAKE_BOOTSTRAP)
    UNSET(CMAKE_USE_SYSTEM_LIBRARIES CACHE)
  ENDIF()

  # Optionally use system utility libraries.
  OPTION(CMAKE_USE_SYSTEM_LIBARCHIVE "Use system-installed libarchive" "${CMAKE_USE_SYSTEM_LIBRARY_LIBARCHIVE}")
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_CURL "Use system-installed curl"
    "${CMAKE_USE_SYSTEM_LIBRARY_CURL}" "NOT CTEST_USE_XMLRPC" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_EXPAT "Use system-installed expat"
    "${CMAKE_USE_SYSTEM_LIBRARY_EXPAT}" "NOT CTEST_USE_XMLRPC" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_ZLIB "Use system-installed zlib"
    "${CMAKE_USE_SYSTEM_LIBRARY_ZLIB}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE;NOT CMAKE_USE_SYSTEM_CURL" ON)
  CMAKE_DEPENDENT_OPTION(CMAKE_USE_SYSTEM_BZIP2 "Use system-installed bzip2"
    "${CMAKE_USE_SYSTEM_LIBRARY_BZIP2}" "NOT CMAKE_USE_SYSTEM_LIBARCHIVE" ON)

  # Mention to the user what system libraries are being used.
  FOREACH(util ${UTILITIES})
    IF(CMAKE_USE_SYSTEM_${util})
      MESSAGE(STATUS "Using system-installed ${util}")
    ENDIF(CMAKE_USE_SYSTEM_${util})
  ENDFOREACH(util)

  # Inform utility library header wrappers whether to use system versions.
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Utilities/cmThirdParty.h.in
    ${CMake_BINARY_DIR}/Utilities/cmThirdParty.h
    @ONLY)

ENDMACRO(CMAKE_HANDLE_SYSTEM_LIBRARIES)




SET(CMAKE_BUILD_ON_VISUAL_STUDIO 0)
IF(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW )
  SET(CMAKE_BUILD_ON_VISUAL_STUDIO 1)
ENDIF(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW )


#-----------------------------------------------------------------------
# a macro to determine the generator and ctest executable to use
# for testing. Simply to improve readability of the main script.
#-----------------------------------------------------------------------
MACRO(CMAKE_SETUP_TESTING)
  IF (NOT DART_ROOT)
    SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
  ENDIF (NOT DART_ROOT)
  
  IF(BUILD_TESTING)
    SET(CMAKE_TEST_GENERATOR "" CACHE STRING 
      "Generator used when running tests")
    SET(CMAKE_TEST_MAKEPROGRAM "" CACHE FILEPATH 
      "Generator used when running tests")
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
    SET(CMAKE_CTEST_COMMAND "${CMake_BIN_DIR}/ctest")
    SET(CMAKE_CMAKE_COMMAND "${CMake_BIN_DIR}/cmake")
    SET(CMAKE_CPACK_COMMAND "${CMake_BIN_DIR}/cpack")
  ENDIF(BUILD_TESTING)

  # configure some files for testing
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/Templates/CTestScript.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CTestScript.cmake"
    @ONLY)
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/.NoDartCoverage
    ${CMake_BINARY_DIR}/Tests/.NoDartCoverage)
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/.NoDartCoverage
    ${CMake_BINARY_DIR}/Modules/.NoDartCoverage)
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/CTestCustom.cmake.in
    ${CMake_BINARY_DIR}/CTestCustom.cmake @ONLY)
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/CTestCustom.ctest.in
    ${CMake_BINARY_DIR}/CTestCustom.ctest @ONLY)
  IF(BUILD_TESTING AND DART_ROOT)
    CONFIGURE_FILE(${CMake_SOURCE_DIR}/CMakeLogo.gif 
      ${CMake_BINARY_DIR}/Testing/HTML/TestingResults/Icons/Logo.gif COPYONLY)
  ENDIF(BUILD_TESTING AND DART_ROOT)
  MARK_AS_ADVANCED(DART_ROOT)
  MARK_AS_ADVANCED(CURL_TESTING)
ENDMACRO(CMAKE_SETUP_TESTING)


# Provide a way for Visual Studio Express users to turn OFF the new FOLDER
# organization feature. Default to ON for non-Express users. Express users must
# explicitly turn off this option to build CMake in the Express IDE...
#
OPTION(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
MARK_AS_ADVANCED(CMAKE_USE_FOLDERS)


#-----------------------------------------------------------------------
# a macro that only sets the FOLDER target property if it's
# "appropriate"
#-----------------------------------------------------------------------
MACRO(CMAKE_SET_TARGET_FOLDER tgt folder)
  IF(CMAKE_USE_FOLDERS)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

    # Really, I just want this to be an "if(TARGET ${tgt})" ...
    # but I'm not sure that our min req'd., CMake 2.4.5 can handle
    # that... so I'm just activating this for now, with a version
    # compare, and only for MSVC builds.
    IF(MSVC)
      IF(NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.8)
        SET_PROPERTY(TARGET "${tgt}" PROPERTY FOLDER "${folder}")
      ENDIF()
    ENDIF()
  ELSE()
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS OFF)
  ENDIF()
ENDMACRO(CMAKE_SET_TARGET_FOLDER)


#-----------------------------------------------------------------------
# a macro to build the utilities used by CMake
# Simply to improve readability of the main script.
#-----------------------------------------------------------------------
MACRO (CMAKE_BUILD_UTILITIES)
  #---------------------------------------------------------------------
  # Create the kwsys library for CMake.
  SET(KWSYS_NAMESPACE cmsys)
  SET(KWSYS_USE_SystemTools 1)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_RegularExpression 1)
  SET(KWSYS_USE_Base64 1)
  SET(KWSYS_USE_MD5 1)
  SET(KWSYS_USE_Process 1)
  SET(KWSYS_USE_CommandLineArguments 1)
  SET(KWSYS_HEADER_ROOT ${CMake_BINARY_DIR}/Source)
  SET(KWSYS_INSTALL_DOC_DIR "${CMake_DOC_DEST}")
  ADD_SUBDIRECTORY(Source/kwsys)
  SET(kwsys_folder "Utilities/KWSys")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE} "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}_c "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}EncodeExecutable "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}ProcessFwd9x "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestDynload "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestProcess "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestsC "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestsCxx "${kwsys_folder}")
  CMAKE_SET_TARGET_FOLDER(${KWSYS_NAMESPACE}TestSharedForward "${kwsys_folder}")

  #---------------------------------------------------------------------
  # Setup third-party libraries.
  # Everything in the tree should be able to include files from the
  # Utilities directory.
  INCLUDE_DIRECTORIES(
    ${CMake_BINARY_DIR}/Utilities
    ${CMake_SOURCE_DIR}/Utilities
    )
  
  # check for the use of system libraries versus builtin ones
  # (a macro defined in this file)
  CMAKE_HANDLE_SYSTEM_LIBRARIES()
  
  #---------------------------------------------------------------------
  # Build zlib library for Curl, CMake, and CTest.
  SET(CMAKE_ZLIB_HEADER "cm_zlib.h")
  IF(CMAKE_USE_SYSTEM_ZLIB)
    FIND_PACKAGE(ZLIB)
    IF(NOT ZLIB_FOUND)
      MESSAGE(FATAL_ERROR 
        "CMAKE_USE_SYSTEM_ZLIB is ON but a zlib is not found!")
    ENDIF(NOT ZLIB_FOUND)
    SET(CMAKE_ZLIB_INCLUDES ${ZLIB_INCLUDE_DIR})
    SET(CMAKE_ZLIB_LIBRARIES ${ZLIB_LIBRARIES})
  ELSE(CMAKE_USE_SYSTEM_ZLIB)
    SET(CMAKE_ZLIB_INCLUDES)
    SET(CMAKE_ZLIB_LIBRARIES cmzlib)
    ADD_SUBDIRECTORY(Utilities/cmzlib)
    CMAKE_SET_TARGET_FOLDER(cmzlib "Utilities/3rdParty")
  ENDIF(CMAKE_USE_SYSTEM_ZLIB)
  
  #---------------------------------------------------------------------
  # Build Curl library for CTest.
  IF(CMAKE_USE_SYSTEM_CURL)
    FIND_PACKAGE(CURL)
    IF(NOT CURL_FOUND)
      MESSAGE(FATAL_ERROR 
        "CMAKE_USE_SYSTEM_CURL is ON but a curl is not found!")
    ENDIF(NOT CURL_FOUND)
    SET(CMAKE_CURL_INCLUDES ${CURL_INCLUDE_DIRS})
    SET(CMAKE_CURL_LIBRARIES ${CURL_LIBRARIES})
  ELSE(CMAKE_USE_SYSTEM_CURL)
    SET(CURL_SPECIAL_ZLIB_H ${CMAKE_ZLIB_HEADER})
    SET(CURL_SPECIAL_LIBZ_INCLUDES ${CMAKE_ZLIB_INCLUDES})
    SET(CURL_SPECIAL_LIBZ ${CMAKE_ZLIB_LIBRARIES})
    OPTION(CMAKE_BUILD_CURL_SHARED "Should curl be built shared" FALSE)
    IF(NOT CMAKE_BUILD_CURL_SHARED)
      ADD_DEFINITIONS(-DCURL_STATICLIB)
    ENDIF(NOT CMAKE_BUILD_CURL_SHARED)
    SET(CMAKE_CURL_INCLUDES)
    SET(CMAKE_CURL_LIBRARIES cmcurl)
    IF(CMAKE_TESTS_CDASH_SERVER)
      SET(CMAKE_CURL_TEST_URL "${CMAKE_TESTS_CDASH_SERVER}/user.php")
    ENDIF(CMAKE_TESTS_CDASH_SERVER)
    ADD_SUBDIRECTORY(Utilities/cmcurl)
    CMAKE_SET_TARGET_FOLDER(cmcurl "Utilities/3rdParty")
    CMAKE_SET_TARGET_FOLDER(LIBCURL "Utilities/3rdParty")
  ENDIF(CMAKE_USE_SYSTEM_CURL)

  #---------------------------------------------------------------------
  # Build Compress library for CTest.
  SET(CMAKE_COMPRESS_INCLUDES 
    "${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmcompress")
  SET(CMAKE_COMPRESS_LIBRARIES "cmcompress")
  ADD_SUBDIRECTORY(Utilities/cmcompress)
  CMAKE_SET_TARGET_FOLDER(cmcompress "Utilities/3rdParty")
  IF(CMAKE_USE_SYSTEM_BZIP2)
    FIND_PACKAGE(BZip2)
  ELSE()
    SET(BZIP2_INCLUDE_DIR 
      "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmbzip2")
    SET(BZIP2_LIBRARIES cmbzip2)
    ADD_SUBDIRECTORY(Utilities/cmbzip2)
    CMAKE_SET_TARGET_FOLDER(cmbzip2 "Utilities/3rdParty")
  ENDIF()

  #---------------------------------------------------------------------
  # Build or use system libarchive for CMake and CTest.
  IF(CMAKE_USE_SYSTEM_LIBARCHIVE)
    IF(EXISTS ${CMAKE_ROOT}/Modules/FindLibArchive.cmake)
      FIND_PACKAGE(LibArchive)
    ELSE()
      CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
      INCLUDE(${CMake_SOURCE_DIR}/Modules/FindLibArchive.cmake)
    ENDIF()
    IF(NOT LibArchive_FOUND)
      MESSAGE(FATAL_ERROR "CMAKE_USE_SYSTEM_LIBARCHIVE is ON but LibArchive is not found!")
    ENDIF()
    SET(CMAKE_TAR_INCLUDES ${LibArchive_INCLUDE_DIRS})
    SET(CMAKE_TAR_LIBRARIES ${LibArchive_LIBRARIES})
  ELSE(CMAKE_USE_SYSTEM_LIBARCHIVE)
    SET(HAVE_LIBZ 1)
    SET(HAVE_ZLIB_H 1)
    SET(ZLIB_INCLUDE_DIR ${CMAKE_ZLIB_INCLUDES})
    SET(ZLIB_LIBRARIES ${CMAKE_ZLIB_LIBRARIES})
    SET(ZLIB_LIBRARY ${CMAKE_ZLIB_LIBRARIES})
    SET(BUILD_ARCHIVE_WITHIN_CMAKE TRUE)
    ADD_DEFINITIONS(-DLIBARCHIVE_STATIC)
    ADD_SUBDIRECTORY(Utilities/cmlibarchive)
    CMAKE_SET_TARGET_FOLDER(cmlibarchive "Utilities/3rdParty")
    SET(CMAKE_TAR_LIBRARIES cmlibarchive ${BZIP2_LIBRARIES})
  ENDIF(CMAKE_USE_SYSTEM_LIBARCHIVE)

  #---------------------------------------------------------------------
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
    SET(CMAKE_EXPAT_INCLUDES)
    SET(CMAKE_EXPAT_LIBRARIES cmexpat)
    ADD_SUBDIRECTORY(Utilities/cmexpat)
    CMAKE_SET_TARGET_FOLDER(cmexpat "Utilities/3rdParty")
  ENDIF(CMAKE_USE_SYSTEM_EXPAT)
  
  #---------------------------------------------------------------------
  # Build XMLRPC library for CMake and CTest.
  IF(CTEST_USE_XMLRPC)
    FIND_PACKAGE(XMLRPC QUIET REQUIRED libwww-client)
    IF(NOT XMLRPC_FOUND)
      MESSAGE(FATAL_ERROR
        "CTEST_USE_XMLRPC is ON but xmlrpc is not found!")
    ENDIF(NOT XMLRPC_FOUND)
    SET(CMAKE_XMLRPC_INCLUDES ${XMLRPC_INCLUDE_DIRS})
    SET(CMAKE_XMLRPC_LIBRARIES ${XMLRPC_LIBRARIES})
  ENDIF(CTEST_USE_XMLRPC)
  
  #---------------------------------------------------------------------
  # Use curses?
  IF (UNIX)
    # there is a bug in the Syllable libraries which makes linking ccmake fail, Alex
    IF(NOT "${CMAKE_SYSTEM_NAME}" MATCHES syllable)
      SET(CURSES_NEED_NCURSES TRUE)
      FIND_PACKAGE(Curses QUIET)
      IF (CURSES_LIBRARY)
        OPTION(BUILD_CursesDialog "Build the CMake Curses Dialog ccmake" ON)
      ELSE (CURSES_LIBRARY)
        MESSAGE("Curses libraries were not found. Curses GUI for CMake will not be built.")
        SET(BUILD_CursesDialog 0)
      ENDIF (CURSES_LIBRARY)
    ELSE(NOT "${CMAKE_SYSTEM_NAME}" MATCHES syllable)
      SET(BUILD_CursesDialog 0)
    ENDIF(NOT "${CMAKE_SYSTEM_NAME}" MATCHES syllable)
  ELSE (UNIX)
    SET(BUILD_CursesDialog 0)
  ENDIF (UNIX)
  IF(BUILD_CursesDialog)
    ADD_SUBDIRECTORY(Source/CursesDialog/form)
  ENDIF(BUILD_CursesDialog)
ENDMACRO (CMAKE_BUILD_UTILITIES)

#-----------------------------------------------------------------------
IF(CMAKE_CXX_PLATFORM_ID MATCHES "OpenBSD")
  EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER}
    ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
    OUTPUT_VARIABLE _GXX_VERSION
  )
  STRING(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
    _GXX_VERSION_SHORT ${_GXX_VERSION})
  IF(_GXX_VERSION_SHORT EQUAL 33)
    MESSAGE(FATAL_ERROR
      "GXX 3.3 on OpenBSD is known to cause CPack to Crash.\n"
      "Please use GXX 4.2 or greater to build CMake on OpenBSD\n"
      "${CMAKE_CXX_COMPILER} version is: ${_GXX_VERSION}")
  ENDIF()
ENDIF()

#-----------------------------------------------------------------------
# The main section of the CMakeLists file
#
#-----------------------------------------------------------------------
# The CMake version number.
SET(CMake_VERSION_MAJOR 2)
SET(CMake_VERSION_MINOR 8)
SET(CMake_VERSION_PATCH 5)
SET(CMake_VERSION_TWEAK 0)
#SET(CMake_VERSION_RC 3)

# Releases define a tweak level.
IF(DEFINED CMake_VERSION_TWEAK)
  SET(CMake_VERSION_IS_RELEASE 1)
  SET(CMake_VERSION_SOURCE "")
ELSE()
  SET(CMake_VERSION_IS_RELEASE 0)

  # Use the date as the tweak level.
  INCLUDE(${CMake_SOURCE_DIR}/Source/kwsys/kwsysDateStamp.cmake)
  SET(CMake_VERSION_TWEAK
    "${KWSYS_DATE_STAMP_YEAR}${KWSYS_DATE_STAMP_MONTH}${KWSYS_DATE_STAMP_DAY}"
    )

  INCLUDE(${CMake_SOURCE_DIR}/Source/CMakeVersionSource.cmake)
ENDIF()

# Compute the full version string.
SET(CMake_VERSION ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH})
IF(${CMake_VERSION_TWEAK} GREATER 0)
  SET(CMake_VERSION ${CMake_VERSION}.${CMake_VERSION_TWEAK})
ENDIF()
IF(CMake_VERSION_RC)
  SET(CMake_VERSION ${CMake_VERSION}-rc${CMake_VERSION_RC})
ENDIF()
IF(CMake_VERSION_SOURCE)
  SET(CMake_VERSION ${CMake_VERSION}-${CMake_VERSION_SOURCE})
ENDIF()

# Include the standard Dart testing module
ENABLE_TESTING()
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

# Set up test-time configuration.
SET_DIRECTORY_PROPERTIES(PROPERTIES
  TEST_INCLUDE_FILE "${CMake_BINARY_DIR}/Tests/EnforceConfig.cmake")

# where to write the resulting executables and libraries
SET(BUILD_SHARED_LIBS OFF)
SET(EXECUTABLE_OUTPUT_PATH "" CACHE INTERNAL "No configurable exe dir.")
SET(LIBRARY_OUTPUT_PATH "" CACHE INTERNAL 
  "Where to put the libraries for CMake")

# The CMake executables usually do not need any rpath to run in the build or
# install tree.
SET(CMAKE_SKIP_RPATH ON CACHE INTERNAL "CMake does not need RPATHs.")

SET(CMAKE_DATA_DIR "/share/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}" CACHE STRING
  "Install location for data (relative to prefix).")
SET(CMAKE_DOC_DIR "/doc/cmake-${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}" CACHE STRING
  "Install location for documentation (relative to prefix).")
SET(CMAKE_MAN_DIR "/man" CACHE STRING
  "Install location for man pages (relative to prefix).")
MARK_AS_ADVANCED(CMAKE_DATA_DIR CMAKE_DOC_DIR CMAKE_MAN_DIR)
IF(CYGWIN AND EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  # Force doc, data and man dirs to conform to cygwin layout.
  SET(CMAKE_DOC_DIR  "/share/doc/cmake-${CMake_VERSION}")
  SET(CMAKE_DATA_DIR "/share/cmake-${CMake_VERSION}")
  SET(CMAKE_MAN_DIR  "/share/man")
  # let the user know we just forced these values
  MESSAGE(STATUS "Setup for Cygwin packaging")
  MESSAGE(STATUS "Override cache CMAKE_DOC_DIR = ${CMAKE_DOC_DIR}")
  MESSAGE(STATUS "Override cache CMAKE_DATA_DIR = ${CMAKE_DATA_DIR}")
  MESSAGE(STATUS "Override cache CMAKE_MAN_DIR = ${CMAKE_MAN_DIR}")
ENDIF()
STRING(REGEX REPLACE "^/" "" CMake_DATA_DEST "${CMAKE_DATA_DIR}")
STRING(REGEX REPLACE "^/" "" CMake_DOC_DEST "${CMAKE_DOC_DIR}")

IF(BUILD_TESTING)
  INCLUDE(${CMake_SOURCE_DIR}/Tests/CMakeInstall.cmake)
ENDIF()

# include special compile flags for some compilers
INCLUDE(CompileFlags.cmake)

# no clue why we are testing for this here
INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(unsetenv "stdlib.h" HAVE_UNSETENV)
CHECK_SYMBOL_EXISTS(environ "stdlib.h" HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE)

# CMAKE_TESTS_CDASH_SERVER: CDash server used by CMake/Tests.
#
# If not defined or "", this variable defaults to the server at
# http://www.cdash.org/CDash.
#
# If set explicitly to "NOTFOUND", curl tests and ctest tests that use
# the network are skipped.
#
# If set to something starting with "http://localhost/", the CDash is
# expected to be an instance of CDash used for CDash testing, pointing
# to a cdash4simpletest database. In these cases, the CDash dashboards
# should be run first.
#
IF("x${CMAKE_TESTS_CDASH_SERVER}" STREQUAL "x")
  SET(CMAKE_TESTS_CDASH_SERVER "http://www.cdash.org/CDash")
ENDIF("x${CMAKE_TESTS_CDASH_SERVER}" STREQUAL "x")

# build the utilities (a macro defined in this file) 
CMAKE_BUILD_UTILITIES()

# On NetBSD ncurses is required, since curses doesn't have the wsyncup()
# function. ncurses is installed via pkgsrc, so the library is in /usr/pkg/lib,
# which isn't in the default linker search path. So without RPATH ccmake 
# doesn't run and the build doesn't succeed since ccmake is executed for
# generating the documentation.
IF(BUILD_CursesDialog)
  GET_FILENAME_COMPONENT(_CURSES_DIR "${CURSES_LIBRARY}" PATH)
  SET(CURSES_NEED_RPATH FALSE)
  IF(NOT "${_CURSES_DIR}" STREQUAL "/lib" AND NOT "${_CURSES_DIR}" STREQUAL "/usr/lib" AND NOT "${_CURSES_DIR}" STREQUAL "/lib64" AND NOT "${_CURSES_DIR}" STREQUAL "/usr/lib64")
    SET(CURSES_NEED_RPATH TRUE)
  ENDIF(NOT "${_CURSES_DIR}" STREQUAL "/lib" AND NOT "${_CURSES_DIR}" STREQUAL "/usr/lib" AND NOT "${_CURSES_DIR}" STREQUAL "/lib64" AND NOT "${_CURSES_DIR}" STREQUAL "/usr/lib64")
ENDIF(BUILD_CursesDialog)

IF(BUILD_QtDialog)
  IF(APPLE)
    SET(CMAKE_BUNDLE_NAME
      "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}-${CMake_VERSION_PATCH}")
    SET(CMAKE_BUNDLE_LOCATION "${CMAKE_INSTALL_PREFIX}")
    # make sure CMAKE_INSTALL_PREFIX ends in /
    STRING(LENGTH "${CMAKE_INSTALL_PREFIX}" LEN)
    MATH(EXPR LEN "${LEN} -1" )
    STRING(SUBSTRING "${CMAKE_INSTALL_PREFIX}" ${LEN} 1 ENDCH)
    IF(NOT "${ENDCH}" STREQUAL "/")
      SET(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/")
    ENDIF(NOT "${ENDCH}" STREQUAL "/")
    SET(CMAKE_INSTALL_PREFIX 
      "${CMAKE_INSTALL_PREFIX}${CMAKE_BUNDLE_NAME}.app/Contents")
  ENDIF(APPLE)
  
  SET(QT_NEED_RPATH FALSE)
  IF(NOT "${QT_LIBRARY_DIR}" STREQUAL "/lib" AND NOT "${QT_LIBRARY_DIR}" STREQUAL "/usr/lib" AND NOT "${QT_LIBRARY_DIR}" STREQUAL "/lib64" AND NOT "${QT_LIBRARY_DIR}" STREQUAL "/usr/lib64")
    SET(QT_NEED_RPATH TRUE)
  ENDIF(NOT "${QT_LIBRARY_DIR}" STREQUAL "/lib" AND NOT "${QT_LIBRARY_DIR}" STREQUAL "/usr/lib" AND NOT "${QT_LIBRARY_DIR}" STREQUAL "/lib64" AND NOT "${QT_LIBRARY_DIR}" STREQUAL "/usr/lib64")
ENDIF(BUILD_QtDialog)


# The same might be true on other systems for other libraries.
# Then only enable RPATH if we have are building at least with cmake 2.4, 
# since this one has much better RPATH features than cmake 2.2.
# The executables are then built with the RPATH for the libraries outside
# the build tree, which is both the build and the install RPATH.
IF (UNIX)
  IF(   CMAKE_USE_SYSTEM_CURL   OR  CMAKE_USE_SYSTEM_ZLIB
        OR  CMAKE_USE_SYSTEM_EXPAT  OR  CTEST_USE_XMLRPC  OR  CURSES_NEED_RPATH  OR  QT_NEED_RPATH)
    SET(CMAKE_SKIP_RPATH OFF CACHE INTERNAL "CMake built with RPATH.")
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  ENDIF(CMAKE_USE_SYSTEM_CURL   OR  CMAKE_USE_SYSTEM_ZLIB
        OR  CMAKE_USE_SYSTEM_EXPAT  OR  CTEST_USE_XMLRPC  OR  CURSES_NEED_RPATH  OR  QT_NEED_RPATH)
ENDIF (UNIX)


# add the uninstall support
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

INCLUDE (CMakeCPack.cmake)

# setup some Testing support (a macro defined in this file)
CMAKE_SETUP_TESTING()
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/DartLocal.conf.in"
  "${CMAKE_CURRENT_BINARY_DIR}/DartLocal.conf"
  COPYONLY)

OPTION(CMAKE_STRICT   
  "Perform strict testing to record property and variable access. Can be used to report any undefined properties or variables" OFF)
MARK_AS_ADVANCED(CMAKE_STRICT)


# build the remaining subdirectories
ADD_SUBDIRECTORY(Source)
ADD_SUBDIRECTORY(Utilities)
ADD_SUBDIRECTORY(Tests)

CMAKE_SET_TARGET_FOLDER(CMakeLibTests "Tests")
CMAKE_SET_TARGET_FOLDER(cmw9xcom "Utilities/Win9xCompat")
CMAKE_SET_TARGET_FOLDER(documentation "Documentation")

# add a test
ADD_TEST(SystemInformationNew "${CMAKE_CMAKE_COMMAND}" 
  --system-information  -G "${CMAKE_TEST_GENERATOR}" )

# Install license file as it requires.
INSTALL(FILES Copyright.txt DESTINATION ${CMake_DOC_DEST})

# Install script directories.
INSTALL(
  DIRECTORY Modules Templates
  DESTINATION "${CMake_DATA_DEST}"
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
  PATTERN "*.sh*" PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE
                              GROUP_READ GROUP_EXECUTE
                              WORLD_READ WORLD_EXECUTE
  PATTERN "CVS" EXCLUDE
  )

#-----------------------------------------------------------------------
# End of the main section of the CMakeLists file
#-----------------------------------------------------------------------

# As a special case when building CMake itself, CMake 2.8.0 and below
# look up EXECUTABLE_OUTPUT_PATH in the top-level CMakeLists.txt file
# to compute the location of the "cmake" executable.  We set it here
# so that those CMake versions can find it.  We wait until after all
# the add_subdirectory() calls to avoid affecting the subdirectories.
SET(EXECUTABLE_OUTPUT_PATH ${CMake_BIN_DIR})
