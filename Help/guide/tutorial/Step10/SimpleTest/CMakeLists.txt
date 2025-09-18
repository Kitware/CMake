# A very simple test framework for demonstrating how dependencies work
cmake_minimum_required(VERSION 3.23)

project(SimpleTest
  VERSION 0.0.1
)

add_library(SimpleTest INTERFACE)
target_sources(SimpleTest
  INTERFACE
    FILE_SET HEADERS
    FILES
      SimpleTest.h
)
target_compile_features(SimpleTest INTERFACE cxx_std_20)

# TODO6: Find the TransitiveDep package with find_package. The SimpleTest
#        build should fail if TransitiveDep cannot be found.

# TODO7: Add the TransitiveDep::TransitiveDep target to the SimpleTest interface
#        library's links. Remember that interface libraries can only have
#        interface properties.

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
  TARGETS SimpleTest
  EXPORT SimpleTestTargets
  FILE_SET HEADERS
)

install(
  EXPORT SimpleTestTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SimpleTest
  NAMESPACE SimpleTest::
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/SimpleTestConfigVersion.cmake
  COMPATIBILITY ExactVersion
  ARCH_INDEPENDENT
)

install(
  FILES
    cmake/simpletest_discover_impl.cmake
    cmake/simpletest_discover_tests.cmake
    cmake/SimpleTestConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleTestConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SimpleTest
)
