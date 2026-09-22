# The setup and cleanup tests of one fixture disagree on the mode.
enable_testing()
add_test(NAME fixture_setup   COMMAND ${CMAKE_COMMAND} -E true)
add_test(NAME fixture_cleanup COMMAND ${CMAKE_COMMAND} -E true)
add_test(NAME test_with_fixture COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(fixture_setup PROPERTIES
  FIXTURES_SETUP MyFixture FIXTURE_REPEAT_MODE AROUND_EACH_REPEAT)
set_tests_properties(fixture_cleanup PROPERTIES
  FIXTURES_CLEANUP MyFixture FIXTURE_REPEAT_MODE AROUND_ALL_REPEATS)
set_tests_properties(test_with_fixture PROPERTIES FIXTURES_REQUIRED MyFixture)
