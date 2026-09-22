# Two fixtures disagree on the mode, and one test takes part in both.
enable_testing()
add_test(NAME setupA   COMMAND ${CMAKE_COMMAND} -E true)
add_test(NAME cleanupA COMMAND ${CMAKE_COMMAND} -E true)
add_test(NAME setupB   COMMAND ${CMAKE_COMMAND} -E true)
add_test(NAME cleanupB COMMAND ${CMAKE_COMMAND} -E true)
add_test(NAME needs_both COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(setupA   PROPERTIES FIXTURES_SETUP A
  FIXTURE_REPEAT_MODE AROUND_EACH_REPEAT)
set_tests_properties(cleanupA PROPERTIES FIXTURES_CLEANUP A
  FIXTURE_REPEAT_MODE AROUND_EACH_REPEAT)
set_tests_properties(setupB   PROPERTIES FIXTURES_SETUP B
  FIXTURE_REPEAT_MODE AROUND_ALL_REPEATS)
set_tests_properties(cleanupB PROPERTIES FIXTURES_CLEANUP B
  FIXTURE_REPEAT_MODE AROUND_ALL_REPEATS)
set_tests_properties(needs_both PROPERTIES FIXTURES_REQUIRED "A;B")
