include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/DefaultLicense.cmake)

install(SBOM licenses EXPORTS licenses DEFAULT_LICENSE MIT DESTINATION .)
