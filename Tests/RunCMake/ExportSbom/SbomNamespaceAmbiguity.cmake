include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceAmbiguity.cmake)

export(SBOM foo_sbom EXPORTS foo)
export(SBOM foo_sbom2 EXPORTS foo)
export(SBOM bar_sbom EXPORTS bar)
