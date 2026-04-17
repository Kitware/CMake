enable_testing()
add_test(NAME test_a COMMAND "${CMAKE_COMMAND}" -E true)
add_test(NAME test_b COMMAND "${CMAKE_COMMAND}" -E true)
set_tests_properties(test_a PROPERTIES MY_PROP "a_val" TIMEOUT 30)
set_tests_properties(test_b PROPERTIES MY_PROP "b_val" TIMEOUT 60)

cmake_language(
  PRINT_PROPERTIES
  TESTS test_a test_b
  NAMED
    MY_PROP
    TIMEOUT
    NOT_SET
)
