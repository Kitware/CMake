project(Selected VERSION 2.3.4 SPDX_LICENSE ISC
  DESCRIPTION "Selected project" LANGUAGES NONE)

include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectLicense.cmake)

install(
  SBOM bundle
  PROJECT bar
  VERSION 1.3.4
  EXPORTS bar_targets child_targets
  DESTINATION .
)

install(SBOM selected PROJECT Selected
  EXPORTS bar_targets child_targets DESTINATION .)
install(SBOM unmatched
  EXPORTS bar_targets child_targets DESTINATION .)
