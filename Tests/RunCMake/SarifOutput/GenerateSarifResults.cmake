# Write some user messages to produce SARIF results
message(WARNING "Example warning message")

# The second warning should be logged, but the rule should not be duplicated
message(WARNING "A second example warning message")

# Status message should not be logged
message(STATUS "Example status message")
