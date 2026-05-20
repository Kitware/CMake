include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")
cmake_policy(SET CMP0219 NEW)

macro(cmp0219_parse_corner_case)
  set(_options)
  set(_one_value_args FOO)
  set(_multi_value_args)
  cmake_parse_arguments(cmp0219_q
                        "${_options}"
                        "${_one_value_args}"
                        "${_multi_value_args}"
                        "${ARGN}")
  cmake_parse_arguments(cmp0219_u
                        "${_options}"
                        "${_one_value_args}"
                        "${_multi_value_args}"
                        ${ARGN})
endmacro()

cmp0219_parse_corner_case(FOO "foo\\;bar")

cmp0219_assert_equal("${cmp0219_q_FOO}" "foo;bar")
cmp0219_assert_undefined(cmp0219_q_UNPARSED_ARGUMENTS)
cmp0219_assert_equal("${cmp0219_u_FOO}" "foo")
cmp0219_assert_equal("${cmp0219_u_UNPARSED_ARGUMENTS}" "bar")

cmp0219_parse_corner_case(FOO "foo\;bar")

cmp0219_assert_equal("${cmp0219_q_FOO}" "foo;bar")
cmp0219_assert_undefined(cmp0219_q_UNPARSED_ARGUMENTS)
cmp0219_assert_equal("${cmp0219_u_FOO}" "foo")
cmp0219_assert_equal("${cmp0219_u_UNPARSED_ARGUMENTS}" "bar")

cmp0219_parse_corner_case(FOO "foo;bar")

cmp0219_assert_equal("${cmp0219_q_FOO}" "foo")
cmp0219_assert_equal("${cmp0219_u_UNPARSED_ARGUMENTS}" "bar")
cmp0219_assert_equal("${cmp0219_u_FOO}" "foo")
cmp0219_assert_equal("${cmp0219_u_UNPARSED_ARGUMENTS}" "bar")

cmp0219_parse_corner_case(FOO foo;bar)

cmp0219_assert_equal("${cmp0219_q_FOO}" "foo")
cmp0219_assert_equal("${cmp0219_u_UNPARSED_ARGUMENTS}" "bar")
cmp0219_assert_equal("${cmp0219_u_FOO}" "foo")
cmp0219_assert_equal("${cmp0219_u_UNPARSED_ARGUMENTS}" "bar")
