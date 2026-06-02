include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

find_package(pybind11 HINTS "${cmp0219_path_native}")

cmp0219_assert_equal("${cmp0219_provider_method}" "FIND_PACKAGE")
cmp0219_assert_equal("${cmp0219_provider_package}" "pybind11")
cmp0219_assert_equal("${cmp0219_provider_argn}" "HINTS;${cmp0219_path_native}")
