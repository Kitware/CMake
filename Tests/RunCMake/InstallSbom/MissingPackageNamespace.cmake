include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MissingPackageNamespace.cmake)

install(SBOM test_targets
  VERSION "1.0.2"
  EXPORT test_targets
  DESTINATION .
)
