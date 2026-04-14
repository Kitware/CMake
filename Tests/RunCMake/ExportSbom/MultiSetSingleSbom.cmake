include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiSetSingleSbom.cmake)

export(EXPORT setA NAMESPACE A:: FILE setA.cmake)
export(EXPORT setB NAMESPACE B:: FILE setB.cmake)
export(SBOM mySbom EXPORTS setA setB)
