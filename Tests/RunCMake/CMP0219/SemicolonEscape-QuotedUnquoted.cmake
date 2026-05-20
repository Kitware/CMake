include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

function(cmp0219_assert_encoded actual expected)
  string(REPLACE "\\" "<BS>" cmp0219_encoded "${actual}")
  string(REPLACE ";" "<SC>" cmp0219_encoded "${cmp0219_encoded}")
  cmp0219_assert_equal("${cmp0219_encoded}" "${expected}")
endfunction()

function(cmp0219_sink out_prefix)
  math(EXPR cmp0219_sink_argc "${ARGC} - 1")
  set(${out_prefix}_argc "${cmp0219_sink_argc}" PARENT_SCOPE)
  if(cmp0219_sink_argc GREATER 0)
    set(${out_prefix}_arg0 "${ARGV1}" PARENT_SCOPE)
  endif()
  if(cmp0219_sink_argc GREATER 1)
    set(${out_prefix}_arg1 "${ARGV2}" PARENT_SCOPE)
  endif()
endfunction()

macro(cmp0219_semicolon_probe prefix value)
  set(${prefix}_named_q "${value}")
  set(${prefix}_named_u ${value})
  set(${prefix}_argv1_q "${ARGV1}")
  set(${prefix}_argv1_u ${ARGV1})
  set(${prefix}_argn_q "${ARGN}")
  set(${prefix}_argn_u ${ARGN})

  cmp0219_sink("${prefix}_sink_named_q" "${value}")
  cmp0219_sink("${prefix}_sink_named_u" ${value})
  cmp0219_sink("${prefix}_sink_argn_q" "${ARGN}")
  cmp0219_sink("${prefix}_sink_argn_u" ${ARGN})
endmacro()

function(cmp0219_run_semicolon_escape_mode mode)
  cmake_policy(SET CMP0219 "${mode}")

  # Value passed with a quoted argument keeps '\;' in textual substitutions
  # until an unquoted use decodes it.
  cmp0219_semicolon_probe("${mode}_quoted" "foo\\;bar" "foo\\;bar")
  cmp0219_assert_encoded("${${mode}_quoted_named_q}" "foo<BS><SC>bar")
  cmp0219_assert_encoded("${${mode}_quoted_named_u}" "foo<SC>bar")
  cmp0219_assert_equal("${${mode}_quoted_sink_named_q_argc}" "1")
  cmp0219_assert_equal("${${mode}_quoted_sink_named_u_argc}" "1")
  cmp0219_assert_equal("${${mode}_quoted_sink_argn_q_argc}" "1")
  cmp0219_assert_equal("${${mode}_quoted_sink_argn_u_argc}" "1")

  # Value passed with an unquoted argument uses '\;' to keep one argument at
  # the call site, but unquoted forwarding splits at ';' in the macro body.
  cmp0219_semicolon_probe("${mode}_unquoted" foo\;bar foo\;bar)
  cmp0219_assert_encoded("${${mode}_unquoted_named_q}" "foo<SC>bar")
  cmp0219_assert_equal("${${mode}_unquoted_sink_named_q_argc}" "1")
  cmp0219_assert_equal("${${mode}_unquoted_sink_named_u_argc}" "2")
  cmp0219_assert_encoded("${${mode}_unquoted_sink_named_u_arg0}" "foo")
  cmp0219_assert_encoded("${${mode}_unquoted_sink_named_u_arg1}" "bar")
  cmp0219_assert_equal("${${mode}_unquoted_sink_argn_q_argc}" "1")
  cmp0219_assert_equal("${${mode}_unquoted_sink_argn_u_argc}" "2")
  cmp0219_assert_encoded("${${mode}_unquoted_sink_argn_u_arg0}" "foo")
  cmp0219_assert_encoded("${${mode}_unquoted_sink_argn_u_arg1}" "bar")
endfunction()

cmp0219_run_semicolon_escape_mode(OLD)
cmp0219_run_semicolon_escape_mode(NEW)
