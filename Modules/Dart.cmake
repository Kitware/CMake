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
  #
  # Section #1:
  #
  # CMake commands that will not vary from project to project. Locates programs
  # on the client and configure site name and build name.
  #

  # find programs used by testing
  # look for the make program
  IF(NOT UNIX) 
    FIND_PROGRAM(MAKEPROGRAM msdev )
  ENDIF(NOT UNIX)
  FIND_PROGRAM(MAKEPROGRAM NAMES gmake make )
  FIND_PROGRAM(CVSCOMMAND cvs )
  FIND_PROGRAM(GREPCOMMAND grep )
  FIND_PROGRAM(COMPRESSIONCOMMAND NAMES gzip compress zip )
  FIND_PROGRAM(GUNZIPCOMMAND gunzip )
  FIND_PROGRAM(JAVACOMMAND java )

  # find a tcl shell command
  IF (UNIX)
    FIND_PROGRAM(TCLSHCOMMAND cygtclsh80 )
  ENDIF(UNIX)
  FIND_PROGRAM(TCLSHCOMMAND 
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
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Utility.conf.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Utility.conf )

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/base64.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/base64.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Build.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Build.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Coverage.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Coverage.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/DashboardManager.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/DashboardManager.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Doxygen.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Doxygen.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/ftp.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/ftp.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Purify.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Purify.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Submit.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Submit.tcl COPYONLY)
  
  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Test.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Test.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Update.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Update.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Client/Utility.tcl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Client/Utility.tcl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/DashboardConfig.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/DashboardConfig.xsl )

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Build.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Build.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Coverage.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Coverage.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/CoverageLog.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/CoverageLog.xsl COPYONLY)
  
  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Dashboard.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Dashboard.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Doxygen.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Doxygen.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Purify.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Purify.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Test.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Test.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/TestOverview.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/TestOverview.xsl COPYONLY)

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/Dart/Source/Server/XSL/Update.xsl.in
  ${PROJECT_BINARY_DIR}/Dart/Source/Server/XSL/Update.xsl COPYONLY)

  #
  # Section 3:
  #
  # Custom targets to perform dashboard builds and submissions.
  # These should NOT need to be modified from project to project.
  #

  # add testing targets
  ADD_CUSTOM_TARGET(Nightly 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly Start Build Test Submit")
  ADD_CUSTOM_TARGET(NightlyBuild   
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly Start Build")
  ADD_CUSTOM_TARGET(NightlyTest 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly Test")
  ADD_CUSTOM_TARGET(NightlyCoverage 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly Coverage")
  ADD_CUSTOM_TARGET(NightlySubmit 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly Submit")
  ADD_CUSTOM_TARGET(NightlyDashboardStart 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly DashboardStart")
  ADD_CUSTOM_TARGET(NightlyDashboardEnd 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Nightly DashboardEnd")
  ADD_CUSTOM_TARGET(ExperimentalBuild 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Experimental Start Build")
  ADD_CUSTOM_TARGET(ExperimentalTest 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Experimental Test")
  ADD_CUSTOM_TARGET(ExperimentalCoverage 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Experimental Coverage")
  ADD_CUSTOM_TARGET(ExperimentalSubmit 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Experimental Submit")
  ADD_CUSTOM_TARGET(Experimental 
  "${TCLSHCOMMAND} Dart/Source/Client/DashboardManager.tcl Experimental Start Build Test Submit")

ENDIF(BUILD_TESTING)

#
# End of Dart.cmake
#
 
