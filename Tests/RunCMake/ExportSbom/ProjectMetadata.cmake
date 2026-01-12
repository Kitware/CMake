include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata.cmake)

export(
  SBOM test_targets
  EXPORT test_targets
  DESCRIPTION "An eloquent description"
  LICENSE "BSD-3"
  HOMEPAGE_URL "www.example.com"
  VERSION "1.3.4"
)
