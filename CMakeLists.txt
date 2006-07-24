PROJECT(CMake)
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# The CMake version number.
SET(CMake_VERSION_MAJOR 2)
SET(CMake_VERSION_MINOR 4)
SET(CMake_VERSION_PATCH 3)
SET(CMake_VERSION "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CMake_VERSION_FULL "${CMake_VERSION}.${CMake_VERSION_PATCH}")

SET(BUILD_SHARED_LIBS OFF)

# The CMake executables do not need any rpath to run in the build or
# install tree.
SET(CMAKE_SKIP_RPATH ON CACHE INTERNAL "CMake does not need RPATHs.")

SET(CMAKE_DATA_DIR "/share/CMake" CACHE STRING 
  "Install location for data (relative to prefix).")
SET(CMAKE_DOC_DIR "/doc/CMake" CACHE STRING
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
# Build zlib library for Curl, CMake, and CTest.
SUBDIRS(Utilities/cmzlib)
SET(CMAKE_ZLIB_INCLUDES
  "${CMAKE_CURRENT_BINARY_DIR}/Utilities"
  )
SET(CMAKE_ZLIB_LIBRARIES "cmzlib")
SET(CURL_SPECIAL_LIBZ ${CMAKE_ZLIB_LIBRARIES})
SET(CURL_SPECIAL_LIBZ_INCLUDES ${CMAKE_ZLIB_INCLUDES})
SET(CURL_SPECIAL_ZLIB_H "cmzlib/zlib.h")

#-----------------------------------------------------------------------------
# Build Curl library for CTest.
SUBDIRS(Utilities/cmcurl)
SET(CMAKE_CURL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/Utilities")
SET(CMAKE_CURL_LIBRARIES "cmcurl")

#-----------------------------------------------------------------------------
# Build Curl library for CTest.
SUBDIRS(Utilities/cmtar)
SET(CMAKE_TAR_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmtar")
SET(CMAKE_TAR_LIBRARIES "cmtar")

#-----------------------------------------------------------------------------
# Build expat library for CMake and CTest.
SUBDIRS(Utilities/cmexpat)
SET(CMAKE_EXPAT_INCLUDES
  "${CMAKE_CURRENT_BINARY_DIR}/Utilities"
  "${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmexpat"
  )
SET(CMAKE_EXPAT_LIBRARIES "cmexpat")

SUBDIRS(Utilities/cmxmlrpc)
SET(CMAKE_XMLRPC_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/cmxmlrpc"
  "${CMAKE_CURRENT_BINARY_DIR}/Utilities/cmxmlrpc"
  )
SET(CMAKE_XMLRPC_LIBRARIES "cmXMLRPC")

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
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/Templates/CTestScript.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CTestScript.cmake"
  @ONLY IMMEDIATE)

