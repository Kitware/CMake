PROJECT(BundleTest)
SET(MACOSX_BUNDLE_INFO_STRING "bundle_info_string")

# Test building a bundle linking to a shared library.
ADD_LIBRARY(BundleTestLib SHARED BundleLib.cxx)
ADD_EXECUTABLE(BundleTest MACOSX_BUNDLE BundleTest.cxx)
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

