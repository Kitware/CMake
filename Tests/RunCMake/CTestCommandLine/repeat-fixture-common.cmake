enable_testing()

add_test(NAME fixture_setup COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(fixture_setup PROPERTIES FIXTURES_SETUP MyFixture)

add_test(NAME test_with_fixture COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(test_with_fixture PROPERTIES FIXTURES_REQUIRED MyFixture)

add_test(NAME fixture_cleanup COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(fixture_cleanup PROPERTIES FIXTURES_CLEANUP MyFixture)

# The mode applies to the whole fixture, so cases that check that setting it
# on one of its tests is enough name only that test in FIXTURE_REPEAT_TESTS.
if(FIXTURE_REPEAT_MODE)
  if(NOT FIXTURE_REPEAT_TESTS)
    set(FIXTURE_REPEAT_TESTS fixture_setup fixture_cleanup)
  endif()
  set_tests_properties(${FIXTURE_REPEAT_TESTS} PROPERTIES
    FIXTURE_REPEAT_MODE ${FIXTURE_REPEAT_MODE})
endif()
