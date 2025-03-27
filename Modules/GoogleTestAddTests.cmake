# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0174 NEW)   # TODO: Remove this when we can update the above to 3.31

function(add_command name test_name)
  set(args "")
  foreach(arg ${ARGN})
    if(arg MATCHES "[^-./:a-zA-Z0-9_]")
      string(APPEND args " [==[${arg}]==]")
    else()
      string(APPEND args " ${arg}")
    endif()
  endforeach()
  string(APPEND script "${name}(${test_name} ${args})\n")
  set(script "${script}" PARENT_SCOPE)
endfunction()

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

function(gtest_discover_tests_impl)

  set(options "")
  set(oneValueArgs
    NO_PRETTY_TYPES   # These two take a value, unlike gtest_discover_tests()
    NO_PRETTY_VALUES  #
    TEST_EXECUTABLE
    TEST_WORKING_DIR
    TEST_PREFIX
    TEST_SUFFIX
    TEST_LIST
    CTEST_FILE
    TEST_DISCOVERY_TIMEOUT
    TEST_XML_OUTPUT_DIR
    # The following are all multi-value arguments in gtest_discover_tests(),
    # but they are each given to us as a single argument. We parse them that
    # way to avoid problems with preserving empty list values and escaping.
    TEST_FILTER
    TEST_EXTRA_ARGS
    TEST_DISCOVERY_EXTRA_ARGS
    TEST_PROPERTIES
    TEST_EXECUTOR
  )
  set(multiValueArgs "")
  cmake_parse_arguments(PARSE_ARGV 0 arg
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
  )

  set(prefix "${arg_TEST_PREFIX}")
  set(suffix "${arg_TEST_SUFFIX}")
  set(script)
  set(suite)
  set(tests)
  set(tests_buffer "")

  # If a file at ${arg_CTEST_FILE} already exists, we overwrite it.
  # For performance reasons, we write to this file in chunks, and this variable
  # is updated to APPEND after the first write.
  set(file_write_mode WRITE)

  if(arg_TEST_FILTER)
    set(filter "--gtest_filter=${arg_TEST_FILTER}")
  else()
    set(filter)
  endif()

  # CMP0178 has already been handled in gtest_discover_tests(), so we only need
  # to implement NEW behavior here. This means preserving empty arguments for
  # TEST_EXECUTOR. For OLD or WARN, gtest_discover_tests() already removed any
  # empty arguments.
  set(launcherArgs "")
  if(NOT "${arg_TEST_EXECUTOR}" STREQUAL "")
    list(JOIN arg_TEST_EXECUTOR "]==] [==[" launcherArgs)
    set(launcherArgs "[==[${launcherArgs}]==]")
  endif()

  # Run test executable to get list of available tests
  if(NOT EXISTS "${arg_TEST_EXECUTABLE}")
    message(FATAL_ERROR
      "Specified test executable does not exist.\n"
      "  Path: '${arg_TEST_EXECUTABLE}'"
    )
  endif()

  set(discovery_extra_args "")
  if(NOT "${arg_TEST_DISCOVERY_EXTRA_ARGS}" STREQUAL "")
    list(JOIN arg_TEST_DISCOVERY_EXTRA_ARGS "]==] [==[" discovery_extra_args)
    set(discovery_extra_args "[==[${discovery_extra_args}]==]")
  endif()

  cmake_language(EVAL CODE
    "execute_process(
      COMMAND ${launcherArgs} [==[${arg_TEST_EXECUTABLE}]==] --gtest_list_tests ${filter} ${discovery_extra_args}
      WORKING_DIRECTORY [==[${arg_TEST_WORKING_DIR}]==]
      TIMEOUT ${arg_TEST_DISCOVERY_TIMEOUT}
      OUTPUT_VARIABLE output
      RESULT_VARIABLE result
    )"
  )
  if(NOT ${result} EQUAL 0)
    string(REPLACE "\n" "\n    " output "${output}")
    if(arg_TEST_EXECUTOR)
      set(path "${arg_TEST_EXECUTOR} ${arg_TEST_EXECUTABLE}")
    else()
      set(path "${arg_TEST_EXECUTABLE}")
    endif()
    message(FATAL_ERROR
      "Error running test executable.\n"
      "  Path: '${path}'\n"
      "  Working directory: '${arg_TEST_WORKING_DIR}'\n"
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
          string(REGEX REPLACE "/[0-9].*" "" pretty_suite "${line}")
          if(NOT arg_NO_PRETTY_TYPES)
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
        if(test MATCHES "#" AND NOT arg_NO_PRETTY_VALUES)
          string(REGEX REPLACE "/[0-9]+[ #]+GetParam\\(\\) = " "/" pretty_test "${test}")
        else()
          string(REGEX REPLACE " +#.*" "" pretty_test "${test}")
        endif()
        string(REGEX REPLACE "^DISABLED_" "" pretty_test "${pretty_test}")
        string(REGEX REPLACE " +#.*" "" test "${test}")
        if(NOT "${arg_TEST_XML_OUTPUT_DIR}" STREQUAL "")
          set(TEST_XML_OUTPUT_PARAM "--gtest_output=xml:${arg_TEST_XML_OUTPUT_DIR}/${prefix}${suite}.${test}${suffix}.xml")
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

        # Add to script. Do not use add_command() here because it messes up the
        # handling of empty values when forwarding arguments, and we need to
        # preserve those carefully for arg_TEST_EXECUTOR and arg_EXTRA_ARGS.
        string(APPEND script "add_test(${guarded_testname} ${launcherArgs}")
        foreach(arg IN ITEMS
          "${arg_TEST_EXECUTABLE}"
          "--gtest_filter=${suite}.${test}"
          "--gtest_also_run_disabled_tests"
          ${TEST_XML_OUTPUT_PARAM}
          )
          if(arg MATCHES "[^-./:a-zA-Z0-9_]")
            string(APPEND script " [==[${arg}]==]")
          else()
            string(APPEND script " ${arg}")
          endif()
        endforeach()
        if(arg_TEST_EXTRA_ARGS)
          list(JOIN arg_TEST_EXTRA_ARGS "]==] [==[" extra_args)
          string(APPEND script " [==[${extra_args}]==]")
        endif()
        string(APPEND script ")\n")

        set(maybe_disabled "")
        if(suite MATCHES "^DISABLED_" OR test MATCHES "^DISABLED_")
          set(maybe_disabled DISABLED TRUE)
        endif()

        add_command(set_tests_properties
          "${guarded_testname}"
          PROPERTIES
          ${maybe_disabled}
          WORKING_DIRECTORY "${arg_TEST_WORKING_DIR}"
          SKIP_REGULAR_EXPRESSION "\\[  SKIPPED \\]"
          ${arg_TEST_PROPERTIES}
        )

        # possibly unbalanced square brackets render lists invalid so skip such
        # tests in ${arg_TEST_LIST}
        if(NOT "${testname}" MATCHES [=[(\[|\])]=])
          # escape ;
          string(REPLACE [[;]] [[\\;]] testname "${testname}")
          list(APPEND tests_buffer "${testname}")
          list(LENGTH tests_buffer tests_buffer_length)
          if(tests_buffer_length GREATER "250")
            # Chunk updates to the final "tests" variable, keeping the
            # "tests_buffer" variable that we append each test to relatively
            # small. This mitigates worsening performance impacts for the
            # corner case of having many thousands of tests.
            list(APPEND tests "${tests_buffer}")
            set(tests_buffer "")
          endif()
        endif()
      endif()

      # If we've built up a sizable script so far, write it out as a chunk now
      # so we don't accumulate a massive string to write at the end
      string(LENGTH "${script}" script_len)
      if(${script_len} GREATER "50000")
        file(${file_write_mode} "${arg_CTEST_FILE}" "${script}")
        set(file_write_mode APPEND)
        set(script "")
      endif()

    endif()
  endforeach()

  if(NOT tests_buffer STREQUAL "")
    list(APPEND tests "${tests_buffer}")
  endif()

  # Create a list of all discovered tests, which users may use to e.g. set
  # properties on the tests
  add_command(set "" ${arg_TEST_LIST} "${tests}")

  # Write remaining content to the CTest script
  file(${file_write_mode} "${arg_CTEST_FILE}" "${script}")

endfunction()

if(CMAKE_SCRIPT_MODE_FILE)
  gtest_discover_tests_impl(
    NO_PRETTY_TYPES ${NO_PRETTY_TYPES}
    NO_PRETTY_VALUES ${NO_PRETTY_VALUES}
    TEST_EXECUTABLE ${TEST_EXECUTABLE}
    TEST_EXECUTOR "${TEST_EXECUTOR}"
    TEST_WORKING_DIR ${TEST_WORKING_DIR}
    TEST_PREFIX ${TEST_PREFIX}
    TEST_SUFFIX ${TEST_SUFFIX}
    TEST_FILTER ${TEST_FILTER}
    TEST_LIST ${TEST_LIST}
    CTEST_FILE ${CTEST_FILE}
    TEST_DISCOVERY_TIMEOUT ${TEST_DISCOVERY_TIMEOUT}
    TEST_XML_OUTPUT_DIR ${TEST_XML_OUTPUT_DIR}
    TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS}"
    TEST_DISCOVERY_EXTRA_ARGS "${TEST_DISCOVERY_EXTRA_ARGS}"
    TEST_PROPERTIES "${TEST_PROPERTIES}"
  )
endif()
