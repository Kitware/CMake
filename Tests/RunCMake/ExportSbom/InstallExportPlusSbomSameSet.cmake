include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InstallExportPlusSbomSameSet.cmake)

export(EXPORT export_set_a NAMESPACE A:: FILE export_set_a.cmake)
export(SBOM mySbom EXPORTS export_set_a)
export(EXPORT export_set_b FILE export_set_b.cmake)
