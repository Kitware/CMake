cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

function(cmp0219_watch_callback variable access value current_list_file stack)
  set(cmp0219_watch_value "${value}" PARENT_SCOPE)
endfunction()

variable_watch(cmp0219_watched cmp0219_watch_callback)
set(cmp0219_watched "${cmp0219_path_native}")

cmp0219_assert_equal("${cmp0219_watch_value}" "${cmp0219_path_native}")
