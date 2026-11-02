include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/UnknownDependencies.cmake)

install(SBOM unknown_dependencies EXPORTS unknown_dependencies DESTINATION .)
