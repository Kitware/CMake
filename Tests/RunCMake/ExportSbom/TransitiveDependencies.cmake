include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/TransitiveDependencies.cmake)

export(SBOM traversal EXPORTS traversal FORMAT spdx)
