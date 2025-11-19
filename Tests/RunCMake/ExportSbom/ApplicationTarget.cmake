include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget.cmake)

export(
  EXPORT application_targets
  SBOM application_targets
  FORMAT "spdx-3.0+json"
)
