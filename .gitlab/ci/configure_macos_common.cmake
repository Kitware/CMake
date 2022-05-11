# Our CI machines do not consistently have Java installed, so a build may
# detect that Java is available and working, but a test machine then not have a
# working Java installed. To work around this, just act as if Java is not
# available on any CI machine.
set(CMake_TEST_Java OFF CACHE BOOL "")

# Qt binaries get placed inside the source directory, which causes them to not
# be included in the install-time rpath, but we still want them in the
# build-time rpath. CMake sets CMAKE_BUILD_WITH_INSTALL_RPATH to ON by default,
# so set it to OFF.
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF CACHE BOOL "")

set(BUILD_QtDialog ON CACHE BOOL "")

# The "XCTest" test uses an explicit deployment target chosen
# when CMake itself is configured.  Use a version that is not
# newer than the macOS version running on any CI host.
set(CMake_TEST_XCTest_DEPLOYMENT_TARGET "10.15" CACHE STRING "")
