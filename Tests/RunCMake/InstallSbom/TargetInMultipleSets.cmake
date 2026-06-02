include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/TargetInMultipleSets.cmake)

install(SBOM mySbom EXPORTS setA1 setA2 DESTINATION .)
