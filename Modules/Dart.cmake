# Dart.cmake
#
# This file configures a project to use the Dart testing/dashboard process.
# It is broken into 3 sections.
# 
# Section #1: Locate programs on the client and determine site and build name
# Section #2: Configure or copy Tcl scripts from the source tree to build tree
# Section #3: Custom targets for performing dashboard builds.
#
#
OPTION(BUILD_TESTING "Build the testing tree." "On")

IF(BUILD_TESTING)
  INCLUDE(${CMAKE_ROOT}/Modules/FindDart.cmake)

  #
  # Section #1:
  #
  # CMake commands that will not vary from project to project. Locates programs
  # on the client and configure site name and build name.
  #

  # the project must have a DartConfig.cmake file
  IF(EXISTS ${PROJECT_SOURCE_DIR}/DartConfig.cmake)
    INCLUDE(${PROJECT_SOURCE_DIR}/DartConfig.cmake)
  ELSE(EXISTS ${PROJECT_SOURCE_DIR}/DartConfig.cmake)
    # Dashboard is opened for submissions for a 24 hour period starting at
    # the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
    SET (NIGHTLY_START_TIME "00:00:00 EDT")

    # Dart server to submit results (used by client)
    SET (DROP_SITE "public.kitware.com")
    SET (DROP_LOCATION "/incoming")
    SET (DROP_SITE_USER "anonymous")
    SET (DROP_SITE_PASSWORD "random@ringworld")
    SET (DROP_SITE_MODE "active")
    SET (TRIGGER_SITE "http://${DROP_SITE}/cgi-bin/Submit-Random-TestingResults.pl")

    # Project Home Page
    SET (PROJECT_URL "http://www.kitware.com")

    # Dart server configuration 
    SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/random-rollup-dashboard.sh")
    #SET (CVS_WEB_URL "")
    #SET (CVS_WEB_CVSROOT "")

    #SET (USE_DOXYGEN "Off")
    #SET (DOXYGEN_URL "" )
  ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/DartConfig.cmake)

  # make program just needs to use CMAKE_MAKE_PROGRAM which is required
  # to be defined by cmake 
  SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
  OPTION(DART_VERBOSE_BUILD "Show the actual output of the build, or if off show a . for each 1024 bytes." 
    OFF)
  OPTION(DART_BUILD_ERROR_REPORT_LIMIT "Limit of reported errors, -1 reports all." -1 )  
  OPTION(DART_BUILD_WARNING_REPORT_LIMIT "Limit of reported warnings, -1 reports all." -1 )  

  SET(VERBOSE_BUILD ${DART_VERBOSE_BUILD})
  SET(BUILD_ERROR_REPORT_LIMIT ${DART_BUILD_ERROR_REPORT_LIMIT})
  SET(BUILD_WARNING_REPORT_LIMIT ${DART_BUILD_WARNING_REPORT_LIMIT})

  FIND_PROGRAM(CVSCOMMAND cvs )
  SET(CVS_UPDATE_OPTIONS "-d -A -P" CACHE STRING "Options passed to the cvs update command.")

  SET(DART_TESTING_TIMEOUT 1500 CACHE STRING "Time alloted for a test before Dart will kill the test.")

  FIND_PROGRAM(COMPRESSIONCOMMAND NAMES gzip compress zip 
    DOC "Path to program used to compress files for transfer to the dart server")
  FIND_PROGRAM(GUNZIPCOMMAND gunzip DOC "Path to gunzip executable")
  FIND_PROGRAM(JAVACOMMAND java DOC "Path to java command, used by the Dart server to create html.")
  FIND_PROGRAM(MEMORYCHECK_COMMAND
    NAMES purify valgrind boundscheck
    PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Rational Software\\Purify\\Setup;InstallFolder]"
    DOC "Path to Rational purify command, used for memory error detection."
    )
  SET(MEMORYCHECK_SUPPRESSIONS_FILE "" CACHE FILEPATH "File that contains suppressions for the memmory checker")
  FIND_PROGRAM(SCPCOMMAND scp DOC "Path to scp command, used by some Dart clients for submitting results to a Dart server (when not using ftp for submissions)")
  FIND_PROGRAM(COVERAGE_COMMAND gcov DOC "Path to the coverage program that Dart client uses for performing coverage inspection")

  # find a tcl shell command
  INCLUDE(${CMAKE_ROOT}/Modules/FindTclsh.cmake)

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
    SET(BUILD_NAME_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
    IF(WIN32)
      SET(BUILD_NAME_SYSTEM_NAME "Win32")
    ENDIF(WIN32)
    IF(UNIX OR BORLAND)
      GET_FILENAME_COMPONENT(DART_CXX_NAME "${CMAKE_CXX_COMPILER}" ${DART_NAME_COMPONENT})
    ELSE(UNIX OR BORLAND)
      GET_FILENAME_COMPONENT(DART_CXX_NAME "${CMAKE_BUILD_TOOL}" ${DART_NAME_COMPONENT})
    ENDIF(UNIX OR BORLAND)
    IF(DART_CXX_NAME MATCHES "msdev")
      SET(DART_CXX_NAME "vs60")
    ENDIF(DART_CXX_NAME MATCHES "msdev")
    IF(DART_CXX_NAME MATCHES "devenv")
      IF(CMAKE_GENERATOR MATCHES "^Visual Studio 7$")
        SET(DART_CXX_NAME "vs70")
      ELSE(CMAKE_GENERATOR MATCHES "^Visual Studio 7$")
        SET(DART_CXX_NAME "vs71")
      ENDIF(CMAKE_GENERATOR MATCHES "^Visual Studio 7$")
    ENDIF(DART_CXX_NAME MATCHES "devenv")
    SET(BUILDNAME "${CMAKE_SYSTEM_NAME}-${DART_CXX_NAME}")
    MESSAGE(STATUS "Using Buildname: ${BUILDNAME}")
  ENDIF(NOT BUILDNAME)
  # set the build command
  BUILD_COMMAND(MAKECOMMAND ${MAKEPROGRAM} )

  SET (DELIVER_CONTINUOUS_EMAIL "Off" CACHE BOOL "Should Dart server send email when build errors are found in Continuous builds?")

  MARK_AS_ADVANCED(
    DART_VERBOSE_BUILD
    DART_BUILD_WARNING_REPORT_LIMIT 
    DART_BUILD_ERROR_REPORT_LIMIT     
    SITE 
    MAKECOMMAND 
    JAVACOMMAND 
    PURIFYCOMMAND
    GUNZIPCOMMAND
    COMPRESSIONCOMMAND
    CVSCOMMAND
    CVS_UPDATE_OPTIONS
    DART_TESTING_TIMEOUT
    SCPCOMMAND
    COVERAGE_COMMAND
    DELIVER_CONTINUOUS_EMAIL
    )
  #  BUILDNAME 

  #
  # Section #2:
  # 
  # Make necessary directories and configure testing scripts
  #

  IF (DART_ROOT)
    # make directories in the binary tree
    FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Testing/HTML/TestingResults/Dashboard
      ${PROJECT_BINARY_DIR}/Testing/HTML/TestingResults/Sites/${SITE}/${BUILDNAME})

    # configure files
    CONFIGURE_FILE(
      ${DART_ROOT}/Source/Client/Dart.conf.in
      ${PROJECT_BINARY_DIR}/DartConfiguration.tcl )

    #
    # Section 3:
    #
    # Custom targets to perform dashboard builds and submissions.
    # These should NOT need to be modified from project to project.
    #

    # add testing targets
    IF(TCL_TCLSH)
      SET(DART_EXPERIMENTAL_NAME Experimental)
      IF(DART_EXPERIMENTAL_USE_PROJECT_NAME)
        SET(DART_EXPERIMENTAL_NAME "${DART_EXPERIMENTAL_NAME}${PROJECT_NAME}")
      ENDIF(DART_EXPERIMENTAL_USE_PROJECT_NAME)
      ADD_CUSTOM_TARGET(${DART_EXPERIMENTAL_NAME}
        ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start Update Configure Build Test)
      ADD_CUSTOM_TARGET(${DART_EXPERIMENTAL_NAME}Submit 
        ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Submit)

      # for non IDE based builds nmake and make 
      # add all these extra targets 
      IF(${CMAKE_MAKE_PROGRAM} MATCHES make)
        # Make targets for Experimental builds
        ADD_CUSTOM_TARGET(ExperimentalStart
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start)
        ADD_CUSTOM_TARGET(ExperimentalUpdate   
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start Update)
        ADD_CUSTOM_TARGET(ExperimentalConfigure   
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start Configure)
        ADD_CUSTOM_TARGET(ExperimentalBuild   
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start Build)
        ADD_CUSTOM_TARGET(ExperimentalTest 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Test)
        ADD_CUSTOM_TARGET(ExperimentalCoverage 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Coverage)
        ADD_CUSTOM_TARGET(ExperimentalDashboardStart 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental DashboardStart)
        ADD_CUSTOM_TARGET(ExperimentalDashboardEnd 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental DashboardEnd)

        # Continuous
        ADD_CUSTOM_TARGET(Continuous 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Start Update Configure Build Test Submit)
        ADD_CUSTOM_TARGET(ContinuousStart
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Start)
        ADD_CUSTOM_TARGET(ContinuousUpdate
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Update)
        ADD_CUSTOM_TARGET(ContinuousConfigure
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Configure)
        ADD_CUSTOM_TARGET(ContinuousBuild   
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Build)
        ADD_CUSTOM_TARGET(ContinuousTest 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Test)
        ADD_CUSTOM_TARGET(ContinuousCoverage 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Coverage)
        ADD_CUSTOM_TARGET(ContinuousSubmit 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Submit)

        # Nightly
        ADD_CUSTOM_TARGET(Nightly 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Start Update Configure Build Test Submit)
        ADD_CUSTOM_TARGET(NightlyStart
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Start)
        ADD_CUSTOM_TARGET(NightlyUpdate
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Update)
        ADD_CUSTOM_TARGET(NightlyConfigure
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Configure)
        ADD_CUSTOM_TARGET(NightlyBuild   
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Build)
        ADD_CUSTOM_TARGET(NightlyTest 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Test)
        ADD_CUSTOM_TARGET(NightlyCoverage 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Coverage)
        ADD_CUSTOM_TARGET(NightlySubmit 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Submit)
        ADD_CUSTOM_TARGET(NightlyDashboardStart 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly DashboardStart)
        ADD_CUSTOM_TARGET(NightlyDashboardEnd 
          ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly DashboardEnd)
      ENDIF (${CMAKE_MAKE_PROGRAM} MATCHES make)

    ELSE(TCL_TCLSH)
      MESSAGE("Could not find TCL_TCLSH, disabling testing." "Error")   
    ENDIF(TCL_TCLSH)
    ENABLE_TESTING()

  ELSE(DART_ROOT)
    # make directories in the binary tree
    FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Testing/Temporary)
    GET_FILENAME_COMPONENT(CMAKE_HOST_PATH ${CMAKE_COMMAND} PATH)
    SET(CMAKE_TARGET_PATH ${EXECUTABLE_OUTPUT_PATH})
    FIND_PROGRAM(CMAKE_CTEST_COMMAND ctest ${CMAKE_HOST_PATH} ${CMAKE_TARGET_PATH})
    MARK_AS_ADVANCED(CMAKE_CTEST_COMMAND)

    # Use CTest
    # configure files
    CONFIGURE_FILE(
      ${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
      ${PROJECT_BINARY_DIR}/DartConfiguration.tcl )

    #
    # Section 3:
    #
    # Custom targets to perform dashboard builds and submissions.
    # These should NOT need to be modified from project to project.
    #

    # add testing targets
    FOREACH(mode Experimental Nightly Continuous NightlyMemoryCheck)
      ADD_CUSTOM_TARGET(${mode} ${CMAKE_CTEST_COMMAND} -D ${mode})
    ENDFOREACH(mode)


    # for non IDE based builds nmake and make 
    # add all these extra targets 
    IF(${CMAKE_MAKE_PROGRAM} MATCHES make)
      # Make targets for Experimental builds
      FOREACH(mode Nightly Experimental Continuous)
        FOREACH(testtype Start Update Configure Build Test Coverage Submit)
          # missing purify
          ADD_CUSTOM_TARGET(${mode}${testtype} 
            ${CMAKE_CTEST_COMMAND} -D ${mode}${testtype})
        ENDFOREACH(testtype)
      ENDFOREACH(mode)
    ENDIF (${CMAKE_MAKE_PROGRAM} MATCHES make)
  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)

#
# End of Dart.cmake
#

