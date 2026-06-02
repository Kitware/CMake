cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

macro(cmp0219_dispatch)
  set(cmp0219_dispatch_argn "${ARGN}")
endmacro()

cmake_language(CALL cmp0219_dispatch HINTS "${cmp0219_path_native}")
cmp0219_assert_equal("${cmp0219_dispatch_argn}" "HINTS;${cmp0219_path_native}")
