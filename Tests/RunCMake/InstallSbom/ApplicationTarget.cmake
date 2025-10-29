include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget.cmake)

install(SBOM application_targets
  EXPORT application_targets
  FORMAT "spdx-3.0+json"
  DESTINATION .
)
