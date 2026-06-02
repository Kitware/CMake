include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

macro(cmp0219_defer_capture var_name)
  set(${var_name} "${ARGN}")
endmacro()

# Deferred call is scheduled with OLD at the call site,
# but executed with NEW at end-of-file.
cmake_policy(SET CMP0219 OLD)
cmake_language(
  DEFER CALL
  cmp0219_defer_capture cmp0219_defer_old_callsite
  HINTS "${cmp0219_path_native}")

# Deferred call is scheduled with NEW at the call site,
# and executed with NEW at end-of-file.
cmake_policy(SET CMP0219 NEW)
cmake_language(
  DEFER CALL
  cmp0219_defer_capture cmp0219_defer_new_callsite
  HINTS "${cmp0219_path_native}")

cmake_language(
  DEFER CALL
  cmp0219_assert_equal "${cmp0219_defer_old_callsite}"
  "HINTS;${cmp0219_path_native}")
cmake_language(
  DEFER CALL
  cmp0219_assert_equal "${cmp0219_defer_new_callsite}"
  "HINTS;${cmp0219_path_native}")
