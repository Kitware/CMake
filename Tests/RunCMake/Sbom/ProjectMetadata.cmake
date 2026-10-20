include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

project(test LANGUAGES C
  DESCRIPTION "Metadata Test Project"
  SPDX_LICENSE BSD-3
  HOMEPAGE_URL www.example.com
  VERSION 1.2.0)

add_library(test INTERFACE)
set_target_properties(test PROPERTIES SPDX_LICENSE license-for-test-target)

add_library(test_targets INTERFACE)
set_target_properties(test_targets PROPERTIES SPDX_LICENSE Apache-2.0)

install(
  TARGETS test test_targets
  EXPORT test_targets
  DESTINATION .
)
