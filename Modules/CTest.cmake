# - setup CTest
# This file configures a project to use the CTest/Dart
# testing/dashboard process.  
OPTION(BUILD_TESTING "Build the testing tree." ON)

IF(BUILD_TESTING)
  # Setup some auxilary macros
  MACRO(SET_IF_NOT_SET var val)
    IF(NOT DEFINED "${var}")
      SET("${var}" "${val}")
    ENDIF(NOT DEFINED "${var}")
  ENDMACRO(SET_IF_NOT_SET)

  MACRO(SET_IF_SET var val)
    IF("${val}" MATCHES "^$")
    ELSE("${val}" MATCHES "^$")
      SET("${var}" "${val}")
    ENDIF("${val}" MATCHES "^$")
  ENDMACRO(SET_IF_SET)

  MACRO(SET_IF_SET_AND_NOT_SET var val)
    IF("${val}" MATCHES "^$")
    ELSE("${val}" MATCHES "^$")
      SET_IF_NOT_SET("${var}" "${val}")
    ENDIF("${val}" MATCHES "^$")
  ENDMACRO(SET_IF_SET_AND_NOT_SET)

  # Make sure testing is enabled
  ENABLE_TESTING()

  IF(EXISTS "${PROJECT_SOURCE_DIR}/CTestConfig.cmake")
    INCLUDE("${PROJECT_SOURCE_DIR}/CTestConfig.cmake")
    SET_IF_SET_AND_NOT_SET(NIGHTLY_START_TIME "${CTEST_NIGHTLY_START_TIME}")
    SET_IF_SET_AND_NOT_SET(DROP_METHOD "${CTEST_DROP_METHOD}")
    SET_IF_SET_AND_NOT_SET(DROP_SITE "${CTEST_DROP_SITE}")
    SET_IF_SET_AND_NOT_SET(DROP_SITE_USER "${CTEST_DROP_SITE_USER}")
    SET_IF_SET_AND_NOT_SET(DROP_SITE_PASSWORD "${CTEST_DROP_SITE_PASWORD}")
    SET_IF_SET_AND_NOT_SET(DROP_SITE_MODE "${CTEST_DROP_SITE_MODE}")
    SET_IF_SET_AND_NOT_SET(DROP_LOCATION "${CTEST_DROP_LOCATION}")
    SET_IF_SET_AND_NOT_SET(TRIGGER_SITE "${CTEST_TRIGGER_SITE}")
  ENDIF(EXISTS "${PROJECT_SOURCE_DIR}/CTestConfig.cmake")

  # the project can have a DartConfig.cmake file
  IF(EXISTS "${PROJECT_SOURCE_DIR}/DartConfig.cmake")
    INCLUDE("${PROJECT_SOURCE_DIR}/DartConfig.cmake")
  ELSE(EXISTS "${PROJECT_SOURCE_DIR}/DartConfig.cmake")

    # Dashboard is opened for submissions for a 24 hour period starting at
    # the specified NIGHTLY_START_TIME. Time is specified in 24 hour format.
    SET_IF_NOT_SET (NIGHTLY_START_TIME "00:00:00 EDT")
    SET_IF_NOT_SET(DROP_METHOD "http")

    # Dart server to submit results (used by client)
    # There should be an option to specify submit method, but I will leave it
    # commented until we decide what to do with it.
    # SET(DROP_METHOD "http" CACHE STRING "Set the CTest submit method. Valid options are http and ftp")
    IF(DROP_METHOD MATCHES http)
      SET_IF_NOT_SET (DROP_SITE "public.kitware.com")
      SET_IF_NOT_SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
    ELSE(DROP_METHOD MATCHES http)
      SET_IF_NOT_SET (DROP_SITE "public.kitware.com")
      SET_IF_NOT_SET (DROP_LOCATION "/incoming")
      SET_IF_NOT_SET (DROP_SITE_USER "anonymous")
      SET_IF_NOT_SET (DROP_SITE_PASSWORD "random@someplace.com")
      SET_IF_NOT_SET (DROP_SITE_MODE "active")
    ENDIF(DROP_METHOD MATCHES http)
    SET_IF_NOT_SET (TRIGGER_SITE "http://${DROP_SITE}/cgi-bin/Submit-Random-TestingResults.cgi")
    SET_IF_NOT_SET (COMPRESS_SUBMISSION ON)

    # Dart server configuration 
    SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/random-rollup-dashboard.sh")
    #SET (CVS_WEB_URL "")
    #SET (CVS_WEB_CVSROOT "")

    #SET (USE_DOXYGEN "Off")
    #SET (DOXYGEN_URL "" )
  ENDIF(EXISTS "${PROJECT_SOURCE_DIR}/DartConfig.cmake")
  SET_IF_NOT_SET (NIGHTLY_START_TIME "00:00:00 EDT")

  # make program just needs to use CMAKE_MAKE_PROGRAM which is required
  # to be defined by cmake 
  SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})

  FIND_PROGRAM(CVSCOMMAND cvs )
  SET(CVS_UPDATE_OPTIONS "-d -A -P" CACHE STRING 
    "Options passed to the cvs update command.")
  FIND_PROGRAM(SVNCOMMAND svn)

  IF(NOT UPDATE_TYPE)
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CVS")
      SET(UPDATE_TYPE cvs)
    ELSE(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CVS")
      IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.svn")
        SET(UPDATE_TYPE svn)
      ENDIF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.svn")
    ENDIF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CVS")
  ENDIF(NOT UPDATE_TYPE)

  IF(NOT UPDATE_TYPE)
    MESSAGE(STATUS "Cannot determine repository type. Please set UPDATE_TYPE to 'cvs' or 'svn'. CTest update will not work.")
  ENDIF(NOT UPDATE_TYPE)

  IF(UPDATE_TYPE MATCHES "[Cc][Vv][Ss]")
    SET(UPDATE_COMMAND "${CVSCOMMAND}")
    SET(UPDATE_OPTIONS "${CVS_UPDATE_OPTIONS}")
  ELSE(UPDATE_TYPE MATCHES "[Cc][Vv][Ss]")
    IF(UPDATE_TYPE MATCHES "[Ss][Vv][Nn]")
      SET(UPDATE_COMMAND "${SVNCOMMAND}")
      SET(UPDATE_OPTIONS "${SVN_UPDATE_OPTIONS}")
    ENDIF(UPDATE_TYPE MATCHES "[Ss][Vv][Nn]")
  ENDIF(UPDATE_TYPE MATCHES "[Cc][Vv][Ss]")

  SET(DART_TESTING_TIMEOUT 1500 CACHE STRING 
    "Maximum time allowed before CTest will kill the test.")

  FIND_PROGRAM(MEMORYCHECK_COMMAND
    NAMES purify valgrind boundscheck
    PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Rational Software\\Purify\\Setup;InstallFolder]"
    DOC "Path to the memory checking command, used for memory error detection."
    )
  SET(MEMORYCHECK_SUPPRESSIONS_FILE "" CACHE FILEPATH 
    "File that contains suppressions for the memory checker")
  FIND_PROGRAM(SCPCOMMAND scp DOC 
    "Path to scp command, used by CTest for submitting results to a Dart server"
    )
  FIND_PROGRAM(COVERAGE_COMMAND gcov DOC 
    "Path to the coverage program that CTest uses for performing coverage inspection"
    )

  # set the site name
  SITE_NAME(SITE)
  # set the build name
  IF(NOT BUILDNAME)
    SET(DART_COMPILER "${CMAKE_CXX_COMPILER}")
    IF(NOT DART_COMPILER)
      SET(DART_COMPILER "${CMAKE_C_COMPILER}")
    ENDIF(NOT DART_COMPILER)
    IF(NOT DART_COMPILER)
      SET(DART_COMPILER "unknown")
    ENDIF(NOT DART_COMPILER)
    IF(WIN32)
      SET(DART_NAME_COMPONENT "NAME_WE")
    ELSE(WIN32)
      SET(DART_NAME_COMPONENT "NAME")
    ENDIF(WIN32)
    IF(NOT BUILD_NAME_SYSTEM_NAME)
      SET(BUILD_NAME_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
    ENDIF(NOT BUILD_NAME_SYSTEM_NAME)
    IF(WIN32)
      SET(BUILD_NAME_SYSTEM_NAME "Win32")
    ENDIF(WIN32)
    IF(UNIX OR BORLAND)
      GET_FILENAME_COMPONENT(DART_CXX_NAME 
        "${CMAKE_CXX_COMPILER}" ${DART_NAME_COMPONENT})
    ELSE(UNIX OR BORLAND)
      GET_FILENAME_COMPONENT(DART_CXX_NAME 
        "${CMAKE_BUILD_TOOL}" ${DART_NAME_COMPONENT})
    ENDIF(UNIX OR BORLAND)
    IF(DART_CXX_NAME MATCHES "msdev")
      SET(DART_CXX_NAME "vs60")
    ENDIF(DART_CXX_NAME MATCHES "msdev")
    IF(DART_CXX_NAME MATCHES "devenv")
      IF(CMAKE_GENERATOR MATCHES "^Visual Studio 7$")
        SET(DART_CXX_NAME "vs70")
      ELSE(CMAKE_GENERATOR MATCHES "^Visual Studio 7$")
        IF(CMAKE_GENERATOR MATCHES "^Visual Studio 7 .NET 2003$")
          SET(DART_CXX_NAME "vs71")
        ELSE(CMAKE_GENERATOR MATCHES "^Visual Studio 7 .NET 2003$")
          SET(DART_CXX_NAME "vs8")
        ENDIF(CMAKE_GENERATOR MATCHES "^Visual Studio 7 .NET 2003$")
      ENDIF(CMAKE_GENERATOR MATCHES "^Visual Studio 7$")
    ENDIF(DART_CXX_NAME MATCHES "devenv")
    SET(BUILDNAME "${BUILD_NAME_SYSTEM_NAME}-${DART_CXX_NAME}")
  ENDIF(NOT BUILDNAME)
  # set the build command
  BUILD_COMMAND(MAKECOMMAND ${MAKEPROGRAM} )

  MARK_AS_ADVANCED(
    COVERAGE_COMMAND
    CVSCOMMAND
    SVNCOMMAND
    CVS_UPDATE_OPTIONS
    SVN_UPDATE_OPTIONS
    MAKECOMMAND 
    MEMORYCHECK_COMMAND
    MEMORYCHECK_SUPPRESSIONS_FILE
    PURIFYCOMMAND
    SCPCOMMAND
    SITE 
    )
  #  BUILDNAME 
  IF(NOT RUN_FROM_DART)
    SET(RUN_FROM_CTEST_OR_DART 1)
    INCLUDE(CTestTargets)
    SET(RUN_FROM_CTEST_OR_DART)
  ENDIF(NOT RUN_FROM_DART)
ENDIF(BUILD_TESTING)
