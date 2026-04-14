include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiSetAmbiguity.cmake)

export(EXPORT setA1 NAMESPACE A1:: FILE setA1.cmake)
export(EXPORT setA2 NAMESPACE A2:: FILE setA2.cmake)
export(SBOM mySbom EXPORTS setB)
