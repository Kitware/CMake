cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

macro(cmp0219_inner)
  set(cmp0219_inner_argn "${ARGN}")
endmacro()

macro(cmp0219_middle)
  cmp0219_inner(${ARGN})
endmacro()

macro(cmp0219_outer)
  cmp0219_middle(${ARGN})
endmacro()

cmp0219_outer(HINTS "${cmp0219_path_native}")
cmp0219_assert_equal("${cmp0219_inner_argn}" "HINTS;${cmp0219_path_native}")
