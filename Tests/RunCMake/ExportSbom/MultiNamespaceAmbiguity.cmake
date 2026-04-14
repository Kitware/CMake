include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiNamespaceAmbiguity.cmake)

export(EXPORT setA NAMESPACE A:: FILE setA-A.cmake)
export(EXPORT setA NAMESPACE OldA:: FILE setA-OldA.cmake)
export(SBOM mySbom EXPORTS setB)
