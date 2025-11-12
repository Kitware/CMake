include("${CMAKE_CURRENT_LIST_DIR}/../cxx-modules-find-bmi.cmake")

report_dirs("${prefix}" "${bmi_destination}")
check_for_bmi("${prefix}" "${bmi_destination}" importable)

report_dirs("${prefix}" "${fs_destination}")
check_for_interface("${prefix}" "${fs_destination}" "" importable.cxx)
