include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceFallback.cmake)

export(SBOM foo EXPORTS foo)
export(SBOM bar EXPORTS bar)
