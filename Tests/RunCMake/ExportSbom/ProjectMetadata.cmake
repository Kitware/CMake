include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata.cmake)

export(
  SBOM test_targets
  EXPORTS test_targets
  DESCRIPTION "An eloquent description"
  DATA_LICENSE "BSD-3"
  HOMEPAGE_URL "www.example.com"
  PACKAGE_URL "https://example.com/test_targets.tar.gz"
  VERSION "1.3.4"
)
