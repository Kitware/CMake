add_test(NAME test COMMAND "${CMAKE_COMMAND}" --help)
set_tests_properties(test PROPERTIES empty "" custom anotherValue)

check_test_property(test custom ..)
