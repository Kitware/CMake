# The fixture's setup test fails on its first run.  With `--repeat until-pass`
# the whole unit runs again, and only the repetition that passed is reported.
cmake_policy(SET CMP0224 NEW)
enable_testing()

set(TEST_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/test_output.txt")
file(WRITE "${TEST_OUTPUT_FILE}" "0")

add_test(NAME fixture_setup COMMAND ${CMAKE_COMMAND}
  "-DTEST_OUTPUT_FILE=${TEST_OUTPUT_FILE}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/test1-pass.cmake")
set_tests_properties(fixture_setup PROPERTIES FIXTURES_SETUP MyFixture)

add_test(NAME test_with_fixture COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(test_with_fixture PROPERTIES FIXTURES_REQUIRED MyFixture)

add_test(NAME fixture_cleanup COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(fixture_cleanup PROPERTIES FIXTURES_CLEANUP MyFixture)
