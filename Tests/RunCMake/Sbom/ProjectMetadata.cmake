include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

project(test LANGUAGES C
  DESCRIPTION "Metadata Test Project"
  SPDX_LICENSE "BSD-3"
  HOMEPAGE_URL "www.example.com"
  VERSION 1.2.0)

add_library(test INTERFACE)

install(
  TARGETS test
  EXPORT test_targets
  DESTINATION .
)
