PROJECT(CMake)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4 FATAL_ERROR) 
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)



#-----------------------------------------------------------------------
# a macro to deal with system libraries, implemented as a macro
# simply to improve readability of the main script
#-----------------------------------------------------------------------
MACRO(CMAKE_HANDLE_SYSTEM_LIBRARIES)
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
      SET(CMAKE_USE_SYSTEM_CURL "${CMAKE_USE_SYSTEM_LIBRARIES}" 
        CACHE BOOL "Use system-installed curl" FORCE)
      SET(CMAKE_USE_SYSTEM_EXPAT "${CMAKE_USE_SYSTEM_LIBRARIES}" 
        CACHE BOOL "Use system-installed expat" FORCE)
      SET(CMAKE_USE_SYSTEM_XMLRPC "${CMAKE_USE_SYSTEM_LIBRARIES}" 
        CACHE BOOL "Use system-installed xmlrpc" FORCE)
      SET(CMAKE_USE_SYSTEM_ZLIB "${CMAKE_USE_SYSTEM_LIBRARIES}" 
        CACHE BOOL "Use system-installed zlib" FORCE)
    ENDIF(CMAKE_USE_SYSTEM_LIBRARIES_USER)
    
    # Optionally use system utility libraries.
    OPTION(CMAKE_USE_SYSTEM_CURL   "Use system-installed curl" 
      ${CMAKE_USE_SYSTEM_LIBRARIES})
    OPTION(CMAKE_USE_SYSTEM_XMLRPC "Use system-installed xmlrpc" 
      ${CMAKE_USE_SYSTEM_LIBRARIES})
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


  # Mention to the user what system libraries are being used.
  FOREACH(util CURL EXPAT XMLRPC ZLIB)
    IF(CMAKE_USE_SYSTEM_${util})
      MESSAGE(STATUS "Using system-installed ${util}")
    ENDIF(CMAKE_USE_SYSTEM_${util})
  ENDFOREACH(util)

  # Inform utility library header wrappers whether to use system versions.
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Utilities/cmThirdParty.h.in
    ${CMake_BINARY_DIR}/Utilities/cmThirdParty.h
    @ONLY)

ENDMACRO(CMAKE_HANDLE_SYSTEM_LIBRARIES)



#-----------------------------------------------------------------------
# a macro to check for MFC and setup to build the MFC Dialog
# simply to improve readability of the main script
#-----------------------------------------------------------------------
MACRO(CMAKE_TEST_FOR_MFC)
  SET(CMAKE_BUILD_ON_VISUAL_STUDIO 0)
  IF(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW )
    SET(CMAKE_BUILD_ON_VISUAL_STUDIO 1)
  ENDIF(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW )
  
  IF(CMAKE_BUILD_ON_VISUAL_STUDIO)
    IF("CMake_HAVE_MFC" MATCHES "^CMake_HAVE_MFC$")
      SET(CHECK_INCLUDE_FILE_VAR "afxwin.h")
      CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.cxx)
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
ENDMACRO(CMAKE_TEST_FOR_MFC)



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
    SET(CMAKE_CTEST_COMMAND "${EXECUTABLE_OUTPUT_PATH}/ctest")
    SET(CMAKE_CMAKE_COMMAND "${EXECUTABLE_OUTPUT_PATH}/cmake")
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
  SUBDIRS(Source/kwsys)
  
  #---------------------------------------------------------------------
  # Setup third-party libraries.
  # Everything in the tree should be able to include files from the
  # Utilities directory.
  INCLUDE_DIRECTORIES(
    ${CMake_SOURCE_DIR}/Utilities
    ${CMake_BINARY_DIR}/Utilities
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
    SUBDIRS(Utilities/cmzlib)
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
    ADD_DEFINITIONS(-DCURL_STATICLIB)
    SET(CMAKE_CURL_INCLUDES)
    SET(CMAKE_CURL_LIBRARIES cmcurl)
    SUBDIRS(Utilities/cmcurl)
  ENDIF(CMAKE_USE_SYSTEM_CURL)

  #---------------------------------------------------------------------
  # Build Tar library for CTest.
  SET(CMTAR_ZLIB_HEADER ${CMAKE_ZLIB_HEADER})
  SET(CMTAR_ZLIB_LIBRARIES ${CMAKE_ZLIB_LIBRARIES})
  SET(CMTAR_ZLIB_INCLUDE_DIRS ${CMAKE_ZLIB_INCLUDES})
  SET(CMAKE_TAR_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmtar)
  SET(CMAKE_TAR_LIBRARIES cmtar)
  SUBDIRS(Utilities/cmtar)
  
  #---------------------------------------------------------------------
  # Build Compress library for CTest.
  SET(CMAKE_COMPRESS_INCLUDES 
    "${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmcompress")
  SET(CMAKE_COMPRESS_LIBRARIES "cmcompress")
  SUBDIRS(Utilities/cmcompress)
  
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
    SUBDIRS(Utilities/cmexpat)
  ENDIF(CMAKE_USE_SYSTEM_EXPAT)
  
  #---------------------------------------------------------------------
  # Build XMLRPC library for CMake and CTest.
  IF(CMAKE_USE_SYSTEM_XMLRPC)
    FIND_PACKAGE(XMLRPC QUIET REQUIRED libwww-client)
    IF(NOT XMLRPC_FOUND)
      MESSAGE(FATAL_ERROR
        "CMAKE_USE_SYSTEM_XMLRPC is ON but a xmlrpc is not found!")
    ENDIF(NOT XMLRPC_FOUND)
    SET(CMAKE_XMLRPC_INCLUDES ${XMLRPC_INCLUDE_DIRS})
    SET(CMAKE_XMLRPC_LIBRARIES ${XMLRPC_LIBRARIES})
  ELSE(CMAKE_USE_SYSTEM_XMLRPC)
    SET(CMAKE_XMLRPC_INCLUDES)
    SET(CMAKE_XMLRPC_LIBRARIES cmXMLRPC)
    SUBDIRS(Utilities/cmxmlrpc)
  ENDIF(CMAKE_USE_SYSTEM_XMLRPC)
  
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
    SUBDIRS(Source/CursesDialog/form)
  ENDIF(BUILD_CursesDialog)
ENDMACRO (CMAKE_BUILD_UTILITIES)



#-----------------------------------------------------------------------
# The main section of the CMakeLists file
#
#-----------------------------------------------------------------------
# The CMake version number.
SET(CMake_VERSION_MAJOR 2)
SET(CMake_VERSION_MINOR 6)
SET(CMake_VERSION_PATCH 3)
#SET(CMake_VERSION_RC 17)
# CVS versions are odd, if this is an odd minor version
# then set the CMake_VERSION_DATE variable
IF("${CMake_VERSION_MINOR}" MATCHES "[13579]$")
  INCLUDE(${CMake_SOURCE_DIR}/Source/kwsys/kwsysDateStamp.cmake)
  SET(CMake_VERSION_DATE
    "${KWSYS_DATE_STAMP_YEAR}${KWSYS_DATE_STAMP_MONTH}${KWSYS_DATE_STAMP_DAY}"
    )
ENDIF("${CMake_VERSION_MINOR}" MATCHES "[13579]$")

SET(CMake_VERSION "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CMake_VERSION_FULL "${CMake_VERSION}.${CMake_VERSION_PATCH}")

# Include the standard Dart testing module
ENABLE_TESTING()
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

# where to write the resulting executables and libraries
SET(BUILD_SHARED_LIBS OFF)
SET(EXECUTABLE_OUTPUT_PATH ${CMake_BINARY_DIR}/bin CACHE INTERNAL 
  "Where to put the executables for CMake")
SET(LIBRARY_OUTPUT_PATH "" CACHE INTERNAL 
  "Where to put the libraries for CMake")
INCLUDE_REGULAR_EXPRESSION("^.*$")

# The CMake executables usually do not need any rpath to run in the build or
# install tree.
SET(CMAKE_SKIP_RPATH ON CACHE INTERNAL "CMake does not need RPATHs.")

SET(CMAKE_DATA_DIR "/share/cmake-${CMake_VERSION}" CACHE STRING
  "Install location for data (relative to prefix).")
SET(CMAKE_DOC_DIR "/doc/cmake-${CMake_VERSION}" CACHE STRING
  "Install location for documentation (relative to prefix).")
SET(CMAKE_MAN_DIR "/man" CACHE STRING
  "Install location for man pages (relative to prefix).")
MARK_AS_ADVANCED(CMAKE_DATA_DIR CMAKE_DOC_DIR CMAKE_MAN_DIR)

# include special compile flags for some compilers
INCLUDE(CompileFlags.cmake)

# no clue why we are testing for this here
INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(unsetenv "stdlib.h" HAVE_UNSETENV)
CHECK_SYMBOL_EXISTS(environ "stdlib.h" HAVE_ENVIRON_NOT_REQUIRE_PROTOTYPE)

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
    IF(CMake_VERSION_DATE)
      SET(CMAKE_BUNDLE_NAME
        "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}-${CMake_VERSION_DATE}")
    ENDIF(CMake_VERSION_DATE)
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


# The same might be true on other systems for other libraries if 
# CMAKE_USE_SYSTEM_XMLRPC or other variables like this are enabled.
# Then only enable RPATH if we have are building at least with cmake 2.4, 
# since this one has much better RPATH features than cmake 2.2.
# The executables are then built with the RPATH for the libraries outside
# the build tree, which is both the build and the install RPATH.
IF (UNIX)
  IF(   CMAKE_USE_SYSTEM_CURL   OR  CMAKE_USE_SYSTEM_ZLIB
        OR  CMAKE_USE_SYSTEM_EXPAT  OR  CMAKE_USE_SYSTEM_XMLRPC  OR  CURSES_NEED_RPATH  OR  QT_NEED_RPATH)
    SET(CMAKE_SKIP_RPATH OFF CACHE INTERNAL "CMake built with RPATH.")
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  ENDIF(CMAKE_USE_SYSTEM_CURL   OR  CMAKE_USE_SYSTEM_ZLIB
        OR  CMAKE_USE_SYSTEM_EXPAT  OR  CMAKE_USE_SYSTEM_XMLRPC  OR  CURSES_NEED_RPATH  OR  QT_NEED_RPATH)
ENDIF (UNIX)


# should we build the MFC dialog? (a macro defined in this file)
CMAKE_TEST_FOR_MFC()

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
SUBDIRS(Source)
SUBDIRS(Modules)
SUBDIRS(Templates)
SUBDIRS(Utilities)
SUBDIRS(Tests)

# add a test
ADD_TEST(SystemInformationNew "${CMAKE_CMAKE_COMMAND}" 
  --system-information  -G "${CMAKE_TEST_GENERATOR}" )

#-----------------------------------------------------------------------
# End of the main section of the CMakeLists file
#-----------------------------------------------------------------------
