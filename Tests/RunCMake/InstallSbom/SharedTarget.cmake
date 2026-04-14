include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SharedTarget.cmake)

install(
  SBOM shared_targets
  EXPORTS shared_targets
  DESTINATION .
)
