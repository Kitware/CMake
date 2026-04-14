include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiNamespaceAmbiguity.cmake)

install(EXPORT setA NAMESPACE A:: DESTINATION lib/cmake/A)
install(EXPORT setA NAMESPACE OldA:: DESTINATION lib/cmake/OldA)
install(SBOM mySbom EXPORTS setB DESTINATION .)
