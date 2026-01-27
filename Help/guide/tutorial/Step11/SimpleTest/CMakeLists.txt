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

# TODO2: Add a compile definition that sets SIMPLETEST_CONFIG=$<CONFIG>
#        on the SimpleTest target

find_package(TransitiveDep REQUIRED)
target_link_libraries(SimpleTest
  INTERFACE
    TransitiveDep::TransitiveDep
)

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
