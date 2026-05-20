cmake_policy(SET CMP0219 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

cmake_policy(PUSH)
cmake_policy(SET CMP0219 OLD)
macro(cmp0219_old_return)
  set(cmp0219_old_return_argn "${ARGN}" PARENT_SCOPE)
  return()
endmacro()

macro(cmp0219_old_set_policy)
  set(cmp0219_old_set_policy_argn "${ARGN}")
  cmake_policy(SET CMP0210 OLD)
endmacro()
cmake_policy(POP)

function(cmp0219_test_return)
  set(cmp0219_return_state "before" PARENT_SCOPE)
  cmp0219_old_return(HINTS "${cmp0219_path_native}")
  set(cmp0219_return_state "after" PARENT_SCOPE)
endfunction()

cmp0219_test_return()
cmp0219_assert_equal("${cmp0219_return_state}" "before")
cmp0219_assert_equal(
  "${cmp0219_old_return_argn}" "HINTS;${cmp0219_path_native}")

cmp0219_old_set_policy(HINTS "${cmp0219_path_native}")
cmake_policy(GET CMP0210 cmp0219_cmp0210_status)
cmp0219_assert_equal("${cmp0219_cmp0210_status}" "OLD")
cmp0219_assert_equal(
  "${cmp0219_old_set_policy_argn}" "HINTS;${cmp0219_path_native}")
