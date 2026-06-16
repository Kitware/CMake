# Write some user messages to produce SARIF results
message(WARNING "Example warning message")

# The second warning should be logged, but the rule should not be duplicated
message(WARNING "A second example warning message")

# Status message should not be logged
message(STATUS "Example status message")

# Test diagnostic reporting
message(AUTHOR_WARNING "Example author warning message")

# Diagnostic results should include a level specific to each reported item
# Change it and issue another one
cmake_diagnostic(SET CMD_AUTHOR SEND_ERROR)
message(AUTHOR_WARNING "Another example author warning message")
