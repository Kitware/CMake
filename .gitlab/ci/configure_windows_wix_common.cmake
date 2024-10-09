get_filename_component(wix3_dir "${CMAKE_CURRENT_LIST_DIR}/../wix3" ABSOLUTE)
set(CMake_TEST_CPACK_WIX3 "${wix3_dir}" CACHE PATH "")

get_filename_component(wix4_dir "${CMAKE_CURRENT_LIST_DIR}/../wix4" ABSOLUTE)
set(CMake_TEST_CPACK_WIX4 "${wix4_dir}" CACHE PATH "")
