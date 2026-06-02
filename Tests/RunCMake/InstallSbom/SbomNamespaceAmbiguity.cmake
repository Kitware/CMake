include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceAmbiguity.cmake)

install(SBOM foo_sbom EXPORTS foo DESTINATION .)
install(SBOM foo_sbom2 EXPORTS foo DESTINATION .)
install(SBOM bar_sbom EXPORTS bar DESTINATION .)
