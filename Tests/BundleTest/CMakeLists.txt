cmake_minimum_required (VERSION 2.6)
PROJECT(BundleTest)
SET(MACOSX_BUNDLE_INFO_STRING "bundle_info_string")
SET(CMAKE_MacOSX_Content_COMPILE_OBJECT "\"${CMAKE_COMMAND}\" -E copy_if_different <SOURCE> <OBJECT>")

ADD_CUSTOM_COMMAND(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  COMMAND /bin/cp
  ARGS "${CMAKE_CURRENT_SOURCE_DIR}/randomResourceFile.plist.in"
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist")

SET_SOURCE_FILES_PROPERTIES(
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
  )

SET_SOURCE_FILES_PROPERTIES(
  SomeRandomFile.txt
  "${CMake_SOURCE_DIR}/ChangeLog.txt"
  PROPERTIES
  MACOSX_PACKAGE_LOCATION MacOS
  )

SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/foobar")

# Test building a bundle linking to a shared library.
ADD_LIBRARY(BundleTestLib SHARED BundleLib.cxx)
ADD_EXECUTABLE(BundleTest
  MACOSX_BUNDLE
  BundleTest.cxx
  SomeRandomFile.txt
  "${CMake_SOURCE_DIR}/ChangeLog.txt"
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  )

TARGET_LINK_LIBRARIES(BundleTest BundleTestLib)

# Test bundle installation.
#INSTALL(TARGETS BundleTestLib DESTINATION Applications/BundleTestExe.app/Contents/Plugins)
INSTALL(TARGETS BundleTestLib DESTINATION Applications/SecondBundleExe.app/Contents/Plugins)
INSTALL(TARGETS BundleTest DESTINATION Applications)

# Test whether bundles respect the output name.  Since the library is
# installed into a location that uses this output name this will fail if the
# bundle does not respect the name.  Also the executable will not be found by
# the test driver if this does not work.
SET_TARGET_PROPERTIES(BundleTest PROPERTIES OUTPUT_NAME BundleTestExe)

# Test executable versioning if it is supported.
IF(NOT XCODE)
  SET_TARGET_PROPERTIES(BundleTest PROPERTIES VERSION 1)
ENDIF(NOT XCODE)

# Make sure the executable can find its installed library.
SET_TARGET_PROPERTIES(BundleTestLib PROPERTIES
  INSTALL_NAME_DIR "@executable_path/../Plugins")

INCLUDE(CPack)

# test the framework find stuff
IF(EXISTS /usr/lib/libtcl.dylib
    AND EXISTS /System/Library/Frameworks/Tcl.framework)
  SET(TCL NOTFOUND)
  FIND_LIBRARY(TCL tcl)
  MESSAGE("frame: ${TCL}")
  IF(NOT "${TCL}" MATCHES .framework)
    MESSAGE(FATAL_ERROR "Could not find tcl framework, found ${TCL}")
  ENDIF(NOT "${TCL}" MATCHES .framework)
  SET(TCL NOTFOUND)
  SET(CMAKE_FIND_FRAMEWORK LAST)
  FIND_LIBRARY(TCL tcl)
  IF("${TCL}" MATCHES .framework)
    MESSAGE(FATAL_ERROR "Found framework and should have found dylib ${TCL}")
  ENDIF("${TCL}" MATCHES .framework)
  SET(TCL NOTFOUND)
  SET(CMAKE_FIND_FRAMEWORK NEVER)
  FIND_LIBRARY(TCL tcl)
  IF("${TCL}" MATCHES .framework)
    MESSAGE(FATAL_ERROR "Found framework and should have found dylib ${TCL}")
  ENDIF("${TCL}" MATCHES .framework)
  MESSAGE("not frame: ${TCL}")
  SET(TCL NOTFOUND)
  SET(CMAKE_FIND_FRAMEWORK FIRST)
  FIND_LIBRARY(TCL tcl)
  IF(NOT "${TCL}" MATCHES .framework)
    MESSAGE(FATAL_ERROR "Could not find tcl framework, found ${TCL}")
  ENDIF(NOT "${TCL}" MATCHES .framework)
  MESSAGE("frame: ${TCL}")
ENDIF(EXISTS /usr/lib/libtcl.dylib
  AND EXISTS /System/Library/Frameworks/Tcl.framework)

SUBDIRS(BundleSubDir)
