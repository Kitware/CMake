include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/DuplicateSbom.cmake)

install(SBOM mySbom EXPORTS setA DESTINATION .)
install(SBOM mySbom EXPORTS setA DESTINATION .)
