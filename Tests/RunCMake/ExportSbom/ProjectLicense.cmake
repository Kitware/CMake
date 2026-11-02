project(Selected VERSION 2.3.4 SPDX_LICENSE ISC
  DESCRIPTION "Selected project" LANGUAGES NONE)

include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectLicense.cmake)

export(SBOM bundle
  PROJECT bar
  VERSION 1.3.4
  EXPORTS bar_targets child_targets)
export(SBOM selected PROJECT Selected
  EXPORTS bar_targets child_targets)
export(SBOM unmatched
  EXPORTS bar_targets child_targets)
