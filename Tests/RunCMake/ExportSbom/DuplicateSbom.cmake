include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/DuplicateSbom.cmake)

export(EXPORT setA FILE setA.cmake)
export(SBOM mySbom EXPORTS setA)
export(SBOM mySbom EXPORTS setA)
