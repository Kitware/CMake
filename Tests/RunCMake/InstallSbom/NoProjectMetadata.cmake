include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/NoProjectMetadata.cmake)

install(SBOM metadata EXPORTS metadata NO_PROJECT_METADATA DESTINATION .)
