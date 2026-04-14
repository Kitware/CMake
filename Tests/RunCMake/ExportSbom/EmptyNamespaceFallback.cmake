include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/EmptyNamespaceFallback.cmake)

export(EXPORT setA FILE setA.cmake)
export(EXPORT setB NAMESPACE B:: FILE setB.cmake)
export(SBOM mySbom EXPORTS setB)
