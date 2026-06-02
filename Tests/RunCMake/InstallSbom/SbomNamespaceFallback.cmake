include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceFallback.cmake)

install(SBOM foo EXPORTS foo DESTINATION .)
install(SBOM bar EXPORTS bar DESTINATION .)
