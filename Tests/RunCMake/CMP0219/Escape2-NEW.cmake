cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

macro(cmp0219_escape str)
  cmp0219_assert_equal("${str}" "\\")
endmacro()

cmp0219_escape("\\")
