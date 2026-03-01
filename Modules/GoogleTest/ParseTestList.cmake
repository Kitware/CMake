# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard(GLOBAL)

function(generate_testname_guards output open_guard_var close_guard_var)
  set(open_guard "[=[")
  set(close_guard "]=]")
  set(counter 1)
  while("${output}" MATCHES "${close_guard}")
    math(EXPR counter "${counter} + 1")
    string(REPEAT "=" ${counter} equals)
    set(open_guard "[${equals}[")
    set(close_guard "]${equals}]")
  endwhile()
  set(${open_guard_var} "${open_guard}" PARENT_SCOPE)
  set(${close_guard_var} "${close_guard}" PARENT_SCOPE)
endfunction()

function(escape_square_brackets output bracket placeholder placeholder_var output_var)
  if("${output}" MATCHES "\\${bracket}")
    set(placeholder "${placeholder}")
    while("${output}" MATCHES "${placeholder}")
        set(placeholder "${placeholder}_")
    endwhile()
    string(REPLACE "${bracket}" "${placeholder}" output "${output}")
    set(${placeholder_var} "${placeholder}" PARENT_SCOPE)
    set(${output_var} "${output}" PARENT_SCOPE)
  endif()
endfunction()

macro(parse_tests_from_output per_test_callback)
  generate_testname_guards("${output}" open_guard close_guard)
  escape_square_brackets("${output}" "[" "__osb" open_sb output)
  escape_square_brackets("${output}" "]" "__csb" close_sb output)

  # Preserve semicolon in test-parameters
  string(REPLACE [[;]] [[\;]] output "${output}")
  string(REPLACE "\n" ";" output "${output}")

  # Command line output doesn't contain information about the file and line number of the tests
  set(current_test_file "")
  set(current_test_line "")

  # Parse output
  foreach(line ${output})
    # Skip header
    if(line MATCHES "gtest_main\\.cc")
      continue()
    endif()

    if(line STREQUAL "")
      continue()
    endif()

    # Do we have a module name or a test name?
    if(NOT line MATCHES "^  ")
      set(current_test_type_param "")

      # Module; remove trailing '.' to get just the name...
      string(REGEX REPLACE "\\.( *#.*)?$" "" current_test_suite "${line}")
      if(line MATCHES "# *TypeParam = (.*)$")
        set(current_test_type_param "${CMAKE_MATCH_1}")
      endif()
    else()
      string(STRIP "${line}" test)
      string(REGEX REPLACE " ( *#.*)?$" "" current_test_name "${test}")

      set(current_test_value_param "")
      if(line MATCHES "# *GetParam\\(\\) = (.*)$")
        set(current_test_value_param "${CMAKE_MATCH_1}")
      endif()

      cmake_language(CALL ${per_test_callback})
    endif()
  endforeach()
endmacro()

macro(get_json_member_with_default json_variable member_name out_variable)
  string(JSON ${out_variable}
    ERROR_VARIABLE error_param
    GET "${${json_variable}}" "${member_name}"
  )
  if(error_param)
    # Member not present
    set(${out_variable} "")
  endif()
endmacro()

macro(parse_tests_from_json json_file per_test_callback)
  if(NOT EXISTS "${json_file}")
    message(FATAL_ERROR "Missing expected JSON file with test list: ${json_file}")
  endif()

  file(READ "${json_file}" test_json)
  string(JSON test_suites_json GET "${test_json}" "testsuites")

  # Return if there are no testsuites
  string(JSON len_test_suites LENGTH "${test_suites_json}")
  if(len_test_suites GREATER 0)
    set(open_sb)
    set(close_sb)

    math(EXPR upper_limit_test_suite_range "${len_test_suites} - 1")

    foreach(index_test_suite RANGE ${upper_limit_test_suite_range})
      string(JSON test_suite_json GET "${test_suites_json}" ${index_test_suite})

      # "suite" is expected to be set in write_test_to_file(). When parsing the
      # plain text output, "suite" is expected to be the original suite name
      # before accounting for pretty names. This may be used to construct the
      # name of XML output results files.
      string(JSON current_test_suite GET "${test_suite_json}" "name")
      string(JSON tests_json GET "${test_suite_json}" "testsuite")

      # Skip test suites without tests
      string(JSON len_tests LENGTH "${tests_json}")
      if(len_tests LESS_EQUAL 0)
        continue()
      endif()

      math(EXPR upper_limit_test_range "${len_tests} - 1")
      foreach(index_test RANGE ${upper_limit_test_range})
        string(JSON test_json GET "${tests_json}" ${index_test})

        string(JSON len_test_parameters LENGTH "${test_json}")
        if(len_test_parameters LESS_EQUAL 0)
          continue()
        endif()

        get_json_member_with_default(test_json "name" current_test_name)
        get_json_member_with_default(test_json "file" current_test_file)
        get_json_member_with_default(test_json "line" current_test_line)
        get_json_member_with_default(test_json "value_param" current_test_value_param)
        get_json_member_with_default(test_json "type_param" current_test_type_param)

        generate_testname_guards(
          "${current_test_suite}${current_test_name}${current_test_value_param}${current_test_type_param}"
          open_guard close_guard
        )
        cmake_language(CALL ${per_test_callback})
      endforeach()
    endforeach()
  endif()
endmacro()
