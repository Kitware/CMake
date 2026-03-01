# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 4.3)

set(launcher "")
if(NOT "${TEST_EXECUTOR}" STREQUAL "")
  list(JOIN TEST_EXECUTOR "]==] [==[" launcher)
  set(launcher "[==[${launcher}]==]")
endif()

set(xml_output_arg "")
if(NOT TEST_XML_OUTPUT STREQUAL "")
  set(xml_output_arg "[==[--gtest_output=xml:${TEST_XML_OUTPUT}]==]")
endif()

set(extra_args "")
if(TEST_EXTRA_ARGS)
  list(JOIN TEST_EXTRA_ARGS "]==] [==[" extra_args)
  set(extra_args "[==[${extra_args}]==]")
endif()

cmake_language(EVAL CODE
  "execute_process(
    COMMAND ${launcher} [==[${TEST_EXECUTABLE}]==]
      [==[--gtest_filter=${TEST_FILTER}]==]
      --gtest_also_run_disabled_tests
      ${xml_output_arg}
      ${extra_args}
    RESULT_VARIABLE result
  )"
)

cmake_language(EXIT ${result})
