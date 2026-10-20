include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateBareLibrary.cmake)

export(SBOM foo EXPORTS foo FORMAT spdx)
