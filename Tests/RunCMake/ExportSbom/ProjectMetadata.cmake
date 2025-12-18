include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata.cmake)

export(
  EXPORT test_targets
  SBOM test_targets
  DESCRIPTION "An eloquent description"
  LICENSE "BSD-3"
  HOMEPAGE_URL "www.example.com"
  VERSION "1.3.4"
)
