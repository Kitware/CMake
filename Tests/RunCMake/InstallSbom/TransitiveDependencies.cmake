include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/TransitiveDependencies.cmake)

install(SBOM traversal EXPORTS traversal FORMAT spdx DESTINATION .)
