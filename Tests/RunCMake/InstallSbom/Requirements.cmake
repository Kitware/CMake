include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Requirements.cmake)

install(SBOM foo EXPORT foo DESTINATION .)
install(SBOM bar EXPORT bar DESTINATION .)
