include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Requirements.cmake)

export(EXPORT foo NAMESPACE foo:: FILE foo.cmake)
export(SBOM foo_sbom EXPORTS foo)
export(SBOM bar_sbom EXPORTS bar)
