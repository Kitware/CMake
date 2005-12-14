# - setup for using Dart.
# This file configures a project to use the Dart testing/dashboard process.
# It is broken into 3 sections.

# 
#  Section #1: Locate programs on the client and determine site and build name
#  Section #2: Configure or copy Tcl scripts from the source tree to build tree
#  Section #3: Custom targets for performing dashboard builds.
#
#
OPTION(BUILD_TESTING "Build the testing tree." ON)

IF(BUILD_TESTING)
  FIND_PACKAGE(Dart)

  #
  # Section #1:
  #
  # CMake commands that will not vary from project to project. Locates programs
  # on the client and configure site name and build name.
  #

  SET(RUN_FROM_DART 1)
  INCLUDE(CTest)
  SET(RUN_FROM_DART)

  # Project Home Page
  SET (PROJECT_URL "http://www.kitware.com")

  FIND_PROGRAM(COMPRESSIONCOMMAND NAMES gzip compress zip 
    DOC "Path to program used to compress files for transfer to the dart server")
  FIND_PROGRAM(GUNZIPCOMMAND gunzip DOC "Path to gunzip executable")
  FIND_PROGRAM(JAVACOMMAND java DOC "Path to java command, used by the Dart server to create html.")
  OPTION(DART_VERBOSE_BUILD "Show the actual output of the build, or if off show a . for each 1024 bytes." 
    OFF)
  OPTION(DART_BUILD_ERROR_REPORT_LIMIT "Limit of reported errors, -1 reports all." -1 )  
  OPTION(DART_BUILD_WARNING_REPORT_LIMIT "Limit of reported warnings, -1 reports all." -1 )  

  SET(VERBOSE_BUILD ${DART_VERBOSE_BUILD})
  SET(BUILD_ERROR_REPORT_LIMIT ${DART_BUILD_ERROR_REPORT_LIMIT})
  SET(BUILD_WARNING_REPORT_LIMIT ${DART_BUILD_WARNING_REPORT_LIMIT})
  SET (DELIVER_CONTINUOUS_EMAIL "Off" CACHE BOOL "Should Dart server send email when build errors are found in Continuous builds?")

  MARK_AS_ADVANCED(
    COMPRESSIONCOMMAND
    DART_BUILD_ERROR_REPORT_LIMIT     
    DART_BUILD_WARNING_REPORT_LIMIT 
    DART_TESTING_TIMEOUT
    DART_VERBOSE_BUILD
    DELIVER_CONTINUOUS_EMAIL
    GUNZIPCOMMAND
    JAVACOMMAND 
    )

  SET(HAVE_DART)
  IF(EXISTS "${DART_ROOT}/Source/Client/Dart.conf.in")
    SET(HAVE_DART 1)
  ENDIF(EXISTS "${DART_ROOT}/Source/Client/Dart.conf.in")

  #
  # Section #2:
  # 
  # Make necessary directories and configure testing scripts
  #
  # find a tcl shell command
  IF(HAVE_DART)
    FIND_PACKAGE(Tclsh)
  ENDIF(HAVE_DART)


  IF (HAVE_DART AND TCL_TCLSH)
    # make directories in the binary tree
    FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/Testing/HTML/TestingResults/Dashboard"
      "${PROJECT_BINARY_DIR}/Testing/HTML/TestingResults/Sites/${SITE}/${BUILDNAME}")

    # configure files
    CONFIGURE_FILE(
      "${DART_ROOT}/Source/Client/Dart.conf.in"
      "${PROJECT_BINARY_DIR}/DartConfiguration.tcl" )

    #
    # Section 3:
    #
    # Custom targets to perform dashboard builds and submissions.
    # These should NOT need to be modified from project to project.
    #

    # add testing targets
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

  ELSE(HAVE_DART AND TCL_TCLSH)
    SET(RUN_FROM_CTEST_OR_DART 1)
    INCLUDE(CTestTargets)
    SET(RUN_FROM_CTEST_OR_DART)
  ENDIF (HAVE_DART AND TCL_TCLSH)
ENDIF(BUILD_TESTING)

#
# End of Dart.cmake
#

