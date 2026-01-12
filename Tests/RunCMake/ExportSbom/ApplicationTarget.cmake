include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget.cmake)

export(
  SBOM application_targets
  EXPORT application_targets
  FORMAT "spdx-3.0+json"
)
