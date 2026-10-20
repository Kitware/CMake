include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateStaticDependency.cmake)

install(SBOM foo EXPORTS foo FORMAT spdx DESTINATION .)
