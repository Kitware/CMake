cmake_policy(PUSH)
cmake_policy(SET CMP0219 OLD)
macro(cmp0219_old_capture)
  set(cmp0219_old_argn "${ARGN}")
endmacro()
cmake_policy(POP)

cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

cmp0219_old_capture(HINTS "${cmp0219_path_native}")
cmp0219_assert_equal("${cmp0219_old_argn}" "HINTS;${cmp0219_path_native}")
