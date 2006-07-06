ADD_CUSTOM_COMMAND(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  COMMAND /bin/cp
  ARGS "${BundleTest_SOURCE_DIR}/randomResourceFile.plist.in"
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist")

SET_SOURCE_FILES_PROPERTIES(
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
  )

SET_SOURCE_FILES_PROPERTIES(
  "${BundleTest_SOURCE_DIR}/SomeRandomFile.txt"
  "${CMake_SOURCE_DIR}/ChangeLog.txt"
  PROPERTIES
  MACOSX_PACKAGE_LOCATION MacOS
  )

ADD_EXECUTABLE(SecondBundle
  MACOSX_BUNDLE
  "${BundleTest_SOURCE_DIR}/BundleTest.cxx"
  "${BundleTest_SOURCE_DIR}/SomeRandomFile.txt"
  "${CMake_SOURCE_DIR}/ChangeLog.txt"
  "${CMAKE_CURRENT_BINARY_DIR}/randomResourceFile.plist"
  )
TARGET_LINK_LIBRARIES(SecondBundle BundleTestLib)
 
# Test bundle installation.
INSTALL(TARGETS SecondBundle DESTINATION Application)

# Test whether bundles respect the output name.  Since the library is
# installed into a location that uses this output name this will fail if the
# bundle does not respect the name.  Also the executable will not be found by
# the test driver if this does not work.
SET_TARGET_PROPERTIES(SecondBundle PROPERTIES OUTPUT_NAME SecondBundleExe)
