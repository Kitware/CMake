# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION})

# Overwrite possibly existing ${_CTEST_FILE} with empty file
set(flush_tests_MODE WRITE)

# Flushes script to ${_CTEST_FILE}
macro(flush_script)
  file(${flush_tests_MODE} "${_CTEST_FILE}" "${script}")
  set(flush_tests_MODE APPEND PARENT_SCOPE)

  set(script "")
endmacro()

# Flushes tests_buffer to tests
macro(flush_tests_buffer)
  list(APPEND tests "${tests_buffer}")
  set(tests_buffer "")
endmacro()

function(add_command NAME TEST_NAME)
  set(args "")
  foreach(arg ${ARGN})
    if(arg MATCHES "[^-./:a-zA-Z0-9_]")
      string(APPEND args " [==[${arg}]==]")
    else()
      string(APPEND args " ${arg}")
    endif()
  endforeach()
  string(APPEND script "${NAME}(${TEST_NAME} ${args})\n")
  string(LENGTH "${script}" script_len)
  if(${script_len} GREATER "50000")
    flush_script()
  endif()
  set(script "${script}" PARENT_SCOPE)
endfunction()

function(generate_testname_guards OUTPUT OPEN_GUARD_VAR CLOSE_GUARD_VAR)
  set(open_guard "[=[")
  set(close_guard "]=]")
  set(counter 1)
  while("${OUTPUT}" MATCHES "${close_guard}")
    math(EXPR counter "${counter} + 1")
    string(REPEAT "=" ${counter} equals)
    set(open_guard "[${equals}[")
    set(close_guard "]${equals}]")
  endwhile()
  set(${OPEN_GUARD_VAR} "${open_guard}" PARENT_SCOPE)
  set(${CLOSE_GUARD_VAR} "${close_guard}" PARENT_SCOPE)
endfunction()

function(escape_square_brackets OUTPUT BRACKET PLACEHOLDER PLACEHOLDER_VAR OUTPUT_VAR)
  if("${OUTPUT}" MATCHES "\\${BRACKET}")
    set(placeholder "${PLACEHOLDER}")
    while("${OUTPUT}" MATCHES "${placeholder}")
        set(placeholder "${placeholder}_")
    endwhile()
    string(REPLACE "${BRACKET}" "${placeholder}" OUTPUT "${OUTPUT}")
    set(${PLACEHOLDER_VAR} "${placeholder}" PARENT_SCOPE)
    set(${OUTPUT_VAR} "${OUTPUT}" PARENT_SCOPE)
  endif()
endfunction()

function(gtest_discover_tests_impl)

  cmake_parse_arguments(
    ""
    ""
    "NO_PRETTY_TYPES;NO_PRETTY_VALUES;TEST_EXECUTABLE;TEST_WORKING_DIR;TEST_PREFIX;TEST_SUFFIX;TEST_LIST;CTEST_FILE;TEST_DISCOVERY_TIMEOUT;TEST_XML_OUTPUT_DIR;TEST_FILTER"
    "TEST_EXTRA_ARGS;TEST_PROPERTIES;TEST_EXECUTOR"
    ${ARGN}
  )

  set(prefix "${_TEST_PREFIX}")
  set(suffix "${_TEST_SUFFIX}")
  set(extra_args ${_TEST_EXTRA_ARGS})
  set(properties ${_TEST_PROPERTIES})
  set(script)
  set(suite)
  set(tests)
  set(tests_buffer)

  if(_TEST_FILTER)
    set(filter "--gtest_filter=${_TEST_FILTER}")
  else()
    set(filter)
  endif()

  # Run test executable to get list of available tests
  if(NOT EXISTS "${_TEST_EXECUTABLE}")
    message(FATAL_ERROR
      "Specified test executable does not exist.\n"
      "  Path: '${_TEST_EXECUTABLE}'"
    )
  endif()
  execute_process(
    COMMAND ${_TEST_EXECUTOR} "${_TEST_EXECUTABLE}" --gtest_list_tests ${filter}
    WORKING_DIRECTORY "${_TEST_WORKING_DIR}"
    TIMEOUT ${_TEST_DISCOVERY_TIMEOUT}
    OUTPUT_VARIABLE output
    RESULT_VARIABLE result
  )
  if(NOT ${result} EQUAL 0)
    string(REPLACE "\n" "\n    " output "${output}")
    if(_TEST_EXECUTOR)
      set(path "${_TEST_EXECUTOR} ${_TEST_EXECUTABLE}")
    else()
      set(path "${_TEST_EXECUTABLE}")
    endif()
    message(FATAL_ERROR
      "Error running test executable.\n"
      "  Path: '${path}'\n"
      "  Result: ${result}\n"
      "  Output:\n"
      "    ${output}\n"
    )
  endif()

  generate_testname_guards("${output}" open_guard close_guard)
  escape_square_brackets("${output}" "[" "__osb" open_sb output)
  escape_square_brackets("${output}" "]" "__csb" close_sb output)
  # Preserve semicolon in test-parameters
  string(REPLACE [[;]] [[\;]] output "${output}")
  string(REPLACE "\n" ";" output "${output}")

  # Parse output
  foreach(line ${output})
    # Skip header
    if(NOT line MATCHES "gtest_main\\.cc")
      # Do we have a module name or a test name?
      if(NOT line MATCHES "^  ")
        # Module; remove trailing '.' to get just the name...
        string(REGEX REPLACE "\\.( *#.*)?$" "" suite "${line}")
        if(line MATCHES "#")
          string(REGEX REPLACE "/.*" "" pretty_suite "${line}")
          if(NOT _NO_PRETTY_TYPES)
            string(REGEX REPLACE ".*/[0-9]+[ .#]+TypeParam = (.*)" "\\1" type_parameter "${line}")
          else()
            string(REGEX REPLACE ".*/([0-9]+)[ .#]+TypeParam = .*" "\\1" type_parameter "${line}")
          endif()
          set(test_name_template "@prefix@@pretty_suite@.@pretty_test@<@type_parameter@>@suffix@")
        else()
          set(pretty_suite "${suite}")
          set(test_name_template "@prefix@@pretty_suite@.@pretty_test@@suffix@")
        endif()
        string(REGEX REPLACE "^DISABLED_" "" pretty_suite "${pretty_suite}")
      else()
        string(STRIP "${line}" test)
        if(test MATCHES "#" AND NOT _NO_PRETTY_VALUES)
          string(REGEX REPLACE "/[0-9]+[ #]+GetParam\\(\\) = " "/" pretty_test "${test}")
        else()
          string(REGEX REPLACE " +#.*" "" pretty_test "${test}")
        endif()
        string(REGEX REPLACE "^DISABLED_" "" pretty_test "${pretty_test}")
        string(REGEX REPLACE " +#.*" "" test "${test}")
        if(NOT "${_TEST_XML_OUTPUT_DIR}" STREQUAL "")
          set(TEST_XML_OUTPUT_PARAM "--gtest_output=xml:${_TEST_XML_OUTPUT_DIR}/${prefix}${suite}.${test}${suffix}.xml")
        else()
          unset(TEST_XML_OUTPUT_PARAM)
        endif()

        string(CONFIGURE "${test_name_template}" testname)
        # unescape []
        if(open_sb)
          string(REPLACE "${open_sb}" "[" testname "${testname}")
        endif()
        if(close_sb)
          string(REPLACE "${close_sb}" "]" testname "${testname}")
        endif()
        set(guarded_testname "${open_guard}${testname}${close_guard}")

        # add to script
        add_command(add_test
          "${guarded_testname}"
          ${_TEST_EXECUTOR}
          "${_TEST_EXECUTABLE}"
          "--gtest_filter=${suite}.${test}"
          "--gtest_also_run_disabled_tests"
          ${TEST_XML_OUTPUT_PARAM}
          ${extra_args}
        )
        if(suite MATCHES "^DISABLED_" OR test MATCHES "^DISABLED_")
          add_command(set_tests_properties
            "${guarded_testname}"
            PROPERTIES DISABLED TRUE
          )
        endif()

        add_command(set_tests_properties
          "${guarded_testname}"
          PROPERTIES
          WORKING_DIRECTORY "${_TEST_WORKING_DIR}"
          SKIP_REGULAR_EXPRESSION "\\[  SKIPPED \\]"
          ${properties}
        )

        # possibly unbalanced square brackets render lists invalid so skip such tests in ${_TEST_LIST}
        if(NOT "${testname}" MATCHES [=[(\[|\])]=])
          # escape ;
          string(REPLACE [[;]] [[\\;]] testname "${testname}")
          list(APPEND tests_buffer "${testname}")
          list(LENGTH tests_buffer tests_buffer_length)
          if(${tests_buffer_length} GREATER "250")
            flush_tests_buffer()
          endif()
        endif()
      endif()
    endif()
  endforeach()


  # Create a list of all discovered tests, which users may use to e.g. set
  # properties on the tests
  flush_tests_buffer()
  add_command(set "" ${_TEST_LIST} "${tests}")

  # Write CTest script
  flush_script()

endfunction()

if(CMAKE_SCRIPT_MODE_FILE)
  gtest_discover_tests_impl(
    NO_PRETTY_TYPES ${NO_PRETTY_TYPES}
    NO_PRETTY_VALUES ${NO_PRETTY_VALUES}
    TEST_EXECUTABLE ${TEST_EXECUTABLE}
    TEST_EXECUTOR ${TEST_EXECUTOR}
    TEST_WORKING_DIR ${TEST_WORKING_DIR}
    TEST_PREFIX ${TEST_PREFIX}
    TEST_SUFFIX ${TEST_SUFFIX}
    TEST_FILTER ${TEST_FILTER}
    TEST_LIST ${TEST_LIST}
    CTEST_FILE ${CTEST_FILE}
    TEST_DISCOVERY_TIMEOUT ${TEST_DISCOVERY_TIMEOUT}
    TEST_XML_OUTPUT_DIR ${TEST_XML_OUTPUT_DIR}
    TEST_EXTRA_ARGS ${TEST_EXTRA_ARGS}
    TEST_PROPERTIES ${TEST_PROPERTIES}
  )
endif()
