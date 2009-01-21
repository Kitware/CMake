PROJECT(BundleGeneratorTest)

CMAKE_MINIMUM_REQUIRED(VERSION 2.7)

# Build a shared library and install it in lib/
ADD_LIBRARY(Library SHARED Library.cxx)
INSTALL(TARGETS Library DESTINATION lib)

# Build an executable and install it in bin/
ADD_EXECUTABLE(Executable Executable.cxx)
TARGET_LINK_LIBRARIES(Executable Library)
INSTALL(TARGETS Executable DESTINATION bin)

# Use the bundle-generator for packaging ...
SET(CPACK_GENERATOR "Bundle")
SET(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/BundleIcon.icns")
SET(CPACK_BUNDLE_NAME "BundleGeneratorTest")
SET(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
SET(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/StartupCommand")
SET(CPACK_PACKAGE_DESCRIPTION "Project for testing OSX bundle generation")

# The custom volume icon is a copy of the normal Mac OSX volume icon, but
# on a white background. This is to differentiate it from the normal one
# so that you can verify that the custom icon is being used by doing a
# visual inspection of the mounted volume... This was added when fixing
# issue #7523...
#
SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/CustomVolumeIcon.icns")

SET(CPACK_PACKAGE_NAME "BundleGeneratorTest")
SET(CPACK_PACKAGE_VERSION "0.1")

INCLUDE(CPack)
