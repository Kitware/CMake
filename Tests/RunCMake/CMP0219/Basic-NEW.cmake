cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

macro(cmp0219_capture package_name first_named)
  set(cmp0219_named "${first_named}")
  set(cmp0219_argn "${ARGN}")
  set(cmp0219_argv "${ARGV}")
  set(cmp0219_argv0 "${ARGV0}")
  set(cmp0219_argv1 "${ARGV1}")
endmacro()

cmp0219_capture(pybind11 HINTS "${cmp0219_path_native}")

cmp0219_assert_equal("${cmp0219_named}" "HINTS")
cmp0219_assert_equal("${cmp0219_argn}" "${cmp0219_path_native}")
cmp0219_assert_equal("${cmp0219_argv}" "pybind11;HINTS;${cmp0219_path_native}")
cmp0219_assert_equal("${cmp0219_argv0}" "pybind11")
cmp0219_assert_equal("${cmp0219_argv1}" "HINTS")
