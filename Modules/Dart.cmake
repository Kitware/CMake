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

  # find programs used by testing
  # look for the make program
  IF(NOT UNIX) 
    FIND_PROGRAM(MAKEPROGRAM msdev )
  ENDIF(NOT UNIX)
  FIND_PROGRAM(MAKEPROGRAM NAMES gmake make )
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
    ADD_CUSTOM_TARGET(Nightly 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Start Build Test Submit)
    ADD_CUSTOM_TARGET(NightlyBuild   
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Start Build)
    ADD_CUSTOM_TARGET(NightlyTest 
    ${TCL_TCLSH} ${DART_ROOT}/Source/Client/DashboardManager.tcl ${PROJECT_BINARY_DIR}/DartConfiguration.tcl Nightly Test)
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
 
