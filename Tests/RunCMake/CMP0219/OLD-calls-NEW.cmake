cmake_policy(PUSH)
cmake_policy(SET CMP0219 NEW)
macro(cmp0219_new_capture)
  set(cmp0219_new_argn "${ARGN}")
endmacro()
cmake_policy(POP)

cmake_policy(SET CMP0219 OLD)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

cmp0219_new_capture(HINTS "${cmp0219_path_native}")
