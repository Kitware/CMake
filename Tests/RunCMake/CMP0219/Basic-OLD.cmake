cmake_policy(SET CMP0219 OLD)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

macro(cmp0219_capture_old)
  set(cmp0219_old_argn "${ARGN}")
endmacro()

cmp0219_capture_old(HINTS "${cmp0219_path_native}")
