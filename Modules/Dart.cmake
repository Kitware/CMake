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

  IF (DART_ROOT)
  #
  # Section #1:
  #
  # CMake commands that will not vary from project to project. Locates programs
  # on the client and configure site name and build name.
  #

  # the project must have a DartConfig.cmake file
  INCLUDE(${PROJECT_SOURCE_DIR}/DartConfig.cmake)

  # make program just needs to use CMAKE_MAKE_PROGRAM which is required
  # to be defined by cmake 
  SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
  FIND_PROGRAM(CVSCOMMAND cvs )
  FIND_PROGRAM(COMPRESSIONCOMMAND NAMES gzip compress zip )
  FIND_PROGRAM(GUNZIPCOMMAND gunzip )
  FIND_PROGRAM(JAVACOMMAND java )
  FIND_PROGRAM(PURIFYCOMMAND purify )

  # find a tcl shell command
  IF (UNIX)
    FIND_PROGRAM(TCL_TCLSH cygtclsh80 )
  ENDIF(UNIX)
  FIND_PROGRAM(TCL_TCLSH 
               NAMES tclsh tclsh83 tclsh8.3 tclsh82 tclsh8.2 tclsh80 tclsh8.0 
               )
  FIND_PROGRAM(HOSTNAME hostname /usr/bsd /usr/sbin /usr/bin /bin /sbin)
  FIND_PROGRAM(NSLOOKUP nslookup /usr/bin /usr/sbin /usr/local/bin)

  # set the site name
  SITE_NAME(SITE)
  # set the build name
  BUILD_NAME(BUILDNAME)
  # set the build command
  BUILD_COMMAND(MAKECOMMAND ${MAKEPROGRAM} )

  #
  # Section #2:
  # 
  # Make necessary directories and configure testing scripts
  #

  # make directories in the binary tree
  MAKE_DIRECTORY(${PROJECT_BINARY_DIR}/Testing/HTML/TestingResults/Dashboard)
  MAKE_DIRECTORY(${PROJECT_BINARY_DIR}/Testing/HTML/TestingResults/Sites/${SITE}/${BUILDNAME})

  # configure files
  CONFIGURE_FILE(
  ${DART_ROOT}/Source/Client/Utility.conf.in
  ${PROJECT_BINARY_DIR}/DartConfiguration.tcl )

  #
  # Section 3:
  #
  # Custom targets to perform dashboard builds and submissions.
  # These should NOT need to be modified from project to project.
  #

  # add testing targets
  IF(TCL_TCLSH)
    ADD_CUSTOM_TARGET(Experimental 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start Build Test)
    ADD_CUSTOM_TARGET(ExperimentalSubmit 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Submit)

  IF (UNIX)
    # Make targets for Experimental builds
    ADD_CUSTOM_TARGET(ExperimentalStart
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start)
    ADD_CUSTOM_TARGET(ExperimentalBuild   
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Start Build)
    ADD_CUSTOM_TARGET(ExperimentalTest 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Test)
    ADD_CUSTOM_TARGET(ExperimentalCoverage 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Experimental Coverage)

    # Continuous
    ADD_CUSTOM_TARGET(Continuous 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Start Update Build Test Submit)
    ADD_CUSTOM_TARGET(ContinuousStart
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Start)
    ADD_CUSTOM_TARGET(ContinuousUpdate
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Continuous Update)
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
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Start Update Build Test Submit)
    ADD_CUSTOM_TARGET(NightlyStart
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Start)
    ADD_CUSTOM_TARGET(NightlyUpdate
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Update)
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
  ENDIF (UNIX)

  ELSE(TCL_TCLSH)
    MESSAGE("Could not find TCL_TCLSH, disabling testing." "Error")   
  ENDIF(TCL_TCLSH)
  ENABLE_TESTING()

  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)

#
# End of Dart.cmake
#
 
