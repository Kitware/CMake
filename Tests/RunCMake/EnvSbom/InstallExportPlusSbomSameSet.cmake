include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InstallExportPlusSbomSameSet.cmake)

install(EXPORT export_set_a NAMESPACE A:: DESTINATION lib/cmake/setA)
install(SBOM mySbom EXPORTS export_set_a DESTINATION .)
install(EXPORT export_set_b DESTINATION lib/cmake/setB)
