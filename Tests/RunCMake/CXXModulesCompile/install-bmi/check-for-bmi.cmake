include("${CMAKE_CURRENT_LIST_DIR}/../cxx-modules-find-bmi.cmake")

report_dirs("${prefix}" "${destination}")
check_for_bmi("${prefix}" "${destination}" importable)
