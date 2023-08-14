enable_testing()

set_property(TEST t DIRECTORY PROPERTY PASS_REGULAR_EXPRESSION "Invalid")
set_property(TEST t DIRECTORY nonexistent PROPERTY PASS_REGULAR_EXPRESSION "Invalid")
