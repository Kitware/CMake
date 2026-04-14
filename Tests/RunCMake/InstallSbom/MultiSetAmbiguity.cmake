include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiSetAmbiguity.cmake)

install(EXPORT setA1 NAMESPACE A1:: DESTINATION lib/cmake/A1)
install(EXPORT setA2 NAMESPACE A2:: DESTINATION lib/cmake/A2)
install(SBOM mySbom EXPORTS setB DESTINATION .)
