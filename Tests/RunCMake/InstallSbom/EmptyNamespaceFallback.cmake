include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/EmptyNamespaceFallback.cmake)

install(EXPORT setA DESTINATION lib/cmake/A)
install(EXPORT setB NAMESPACE B:: DESTINATION lib/cmake/B)
install(SBOM mySbom EXPORTS setB DESTINATION .)
