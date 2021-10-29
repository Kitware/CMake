execute_process(
  COMMAND ${CMAKE_COMMAND} -E environment
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  RESULT_VARIABLE res)

if (res)
  message(FATAL_ERROR "Failed with exit code ${res}: ${err}")
endif ()

if (CMAKE_HOST_WIN32)
  set(path_sep ";")
else ()
  set(path_sep ":")
endif ()

set(unexpect_SET_FROM_AMBIENT_unset "")
set(unexpect_SET_FROM_ENVIRONMENT_PROPERTY_unset "")
set(unexpect_UNSET_EXPLICIT "")
set(unexpect_UNSET_VIA_RESET "")
set(expect_DIRECT "new")
set(expect_STRING_MANIP "prefix-pre-core-post-suffix")
set(expect_PATH_MANIP "prefix${path_sep}pre${path_sep}core${path_sep}post${path_sep}suffix")
set(expect_CMAKE_LIST_MANIP "prefix;pre;core;post;suffix")
set(expect_STRING_DNE "prefix-prepost-suffix")
set(expect_PATH_DNE "prefix${path_sep}pre${path_sep}post${path_sep}suffix")
set(expect_CMAKE_LIST_DNE "prefix;pre;post;suffix")
set(expect_SET_FROM_AMBIENT_replace "new")
set(expect_SET_FROM_AMBIENT_string "basenew")
set(expect_SET_FROM_AMBIENT_path "base${path_sep}new")
set(expect_SET_FROM_AMBIENT_list "base;new")
set(expect_SET_FROM_ENVIRONMENT_PROPERTY_replace "new")
set(expect_SET_FROM_ENVIRONMENT_PROPERTY_string "basenew")
set(expect_SET_FROM_ENVIRONMENT_PROPERTY_path "base${path_sep}new")
set(expect_SET_FROM_ENVIRONMENT_PROPERTY_list "base;new")

set(expected_vars
  SET_FROM_AMBIENT_replace
  SET_FROM_AMBIENT_string
  SET_FROM_AMBIENT_path
  SET_FROM_AMBIENT_list
  SET_FROM_ENVIRONMENT_PROPERTY_replace
  SET_FROM_ENVIRONMENT_PROPERTY_string
  SET_FROM_ENVIRONMENT_PROPERTY_path
  SET_FROM_ENVIRONMENT_PROPERTY_list
  DIRECT
  STRING_MANIP
  PATH_MANIP
  CMAKE_LIST_MANIP
  STRING_DNE
  PATH_DNE
  CMAKE_LIST_DNE)

while (out)
  string(FIND "${out}" "\n" nl_pos)
  string(SUBSTRING "${out}" 0 "${nl_pos}" line)
  math(EXPR line_next "${nl_pos} + 1")
  string(SUBSTRING "${out}" "${line_next}" -1 out)

  string(FIND "${line}" "=" eq_pos)
  string(SUBSTRING "${line}" 0 "${eq_pos}" name)
  math(EXPR value_start "${eq_pos} + 1")
  string(SUBSTRING "${line}" "${value_start}" -1 value)

  if (DEFINED "unexpect_${name}")
    message(SEND_ERROR "Found `${name}=${value}` when it should have been unset")
  elseif (DEFINED "expect_${name}")
    list(REMOVE_ITEM expected_vars "${name}")
    if (expect_${name} STREQUAL value)
      message(STATUS "Found `${name}=${value}` as expected")
    else ()
      message(SEND_ERROR "Found `${name}=${value}` when it should have been ${expect_${name}}")
    endif ()
  endif ()
endwhile ()

if (expected_vars)
  message(SEND_ERROR "Did not test expected variables: ${expected_vars}")
endif ()
