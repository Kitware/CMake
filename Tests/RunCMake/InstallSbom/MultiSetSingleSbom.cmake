include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiSetSingleSbom.cmake)

install(EXPORT setA NAMESPACE A:: DESTINATION lib/cmake/A)
install(EXPORT setB NAMESPACE B:: DESTINATION lib/cmake/B)
install(SBOM mySbom EXPORTS setA setB DESTINATION .)
