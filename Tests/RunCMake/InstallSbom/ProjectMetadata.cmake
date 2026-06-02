include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata.cmake)

install(
  SBOM test_targets
  DESCRIPTION "An eloquent description"
  LICENSE "BSD-3"
  HOMEPAGE_URL "www.example.com"
  PACKAGE_URL "https://example.com/test_targets.tar.gz"
  VERSION "1.3.4"
  EXPORTS test_targets
  DESTINATION .
)
