include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateBareLibrary.cmake)

install(SBOM foo EXPORTS foo FORMAT spdx DESTINATION .)
