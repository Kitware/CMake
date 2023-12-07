enable_testing()

set_tests_properties(t DIRECTORY PROPERTIES PASS_REGULAR_EXPRESSION "Top directory")
set_tests_properties(t DIRECTORY nonexistent PROPERTIES PASS_REGULAR_EXPRESSION "Top directory")
