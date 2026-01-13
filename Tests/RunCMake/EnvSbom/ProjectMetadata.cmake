project(test_project LANGUAGES NONE
  VERSION 1.3
  DESCRIPTION "An eloquent description"
  HOMEPAGE_URL "www.example.com"
  SPDX_LICENSE "BSD-3")
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata.cmake)
