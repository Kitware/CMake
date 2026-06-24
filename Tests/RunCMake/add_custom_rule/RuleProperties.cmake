include(check_property.cmake)

unset(TEST_FAILED)


add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 NAME "rule1")
check_rule_property(rule1 PARENT_RULE "")
check_rule_property(rule1 COMMAND_EXPAND_LISTS "1")
check_rule_property(rule1 VERBATIM "1")
check_rule_property(rule1 OUTPUT_FILE_SET "__cmake_rule_<RULE>_<TARGET>_<FILE_SET>_outputs;SOURCES")

add_custom_rule(rule2 FROM_RULE rule1)
check_rule_property(rule2 NAME "rule2")
check_rule_property(rule2 PARENT_RULE "rule1")
check_rule_property(rule2 COMMAND_EXPAND_LISTS "1")
check_rule_property(rule2 VERBATIM "1")
check_rule_property(rule1 OUTPUT_FILE_SET "__cmake_rule_<RULE>_<TARGET>_<FILE_SET>_outputs;SOURCES")

set_property(RULE rule1 PROPERTY COMMAND_EXPAND_LISTS "0")
set_property(RULE rule1 PROPERTY VERBATIM "0")
add_custom_rule(rule3 FROM_RULE rule1)
check_rule_property(rule3 NAME "rule3")
check_rule_property(rule3 PARENT_RULE "rule1")
check_rule_property(rule3 COMMAND_EXPAND_LISTS "0")
check_rule_property(rule3 VERBATIM "0")

add_custom_rule(rule4 FROM_RULE rule2)
check_rule_property(rule4 NAME "rule4")
check_rule_property(rule4 PARENT_RULE "rule2")


if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
