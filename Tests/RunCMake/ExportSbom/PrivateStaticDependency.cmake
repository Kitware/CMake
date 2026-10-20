include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateStaticDependency.cmake)

export(SBOM foo EXPORTS foo FORMAT spdx)
