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
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  )

TARGET_LINK_LIBRARIES(BundleTest BundleTestLib)

# Test bundle installation.
INSTALL(TARGETS BundleTestLib DESTINATION Application/BundleTestExe.app/Contents/Plugins)
INSTALL(TARGETS BundleTest DESTINATION Application)

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

