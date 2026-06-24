
include(check_property.cmake)

unset(TEST_FAILED)

add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 COMMAND "cmd;arg1;arg2")
check_rule_property(rule1 COMMAND_COUNT "1")
check_rule_property(rule1 COMMAND_0 "cmd;arg1;arg2")
check_rule_property(rule1 COMMAND_1 "NOTFOUND")

add_custom_rule(rule2 OUTPUT out1 COMMAND cmd1 arg1 arg2  COMMAND cmd2 arg1 arg2)
check_rule_property(rule2 COMMAND "cmd1;arg1;arg2")
check_rule_property(rule2 COMMAND_COUNT "2")
check_rule_property(rule2 COMMAND_0 "cmd1;arg1;arg2")
check_rule_property(rule2 COMMAND_1 "cmd2;arg1;arg2")
check_rule_property(rule2 COMMAND_2 "NOTFOUND")

add_custom_rule(rule3 OUTPUT out1 COMMAND cmd1 arg1 arg2  OUTPUT out2 COMMAND cmd2 arg1 arg2)
check_rule_property(rule3 COMMAND "cmd1;arg1;arg2")
check_rule_property(rule3 COMMAND_COUNT "2")
check_rule_property(rule3 COMMAND_0 "cmd1;arg1;arg2")
check_rule_property(rule3 COMMAND_1 "cmd2;arg1;arg2")
check_rule_property(rule3 COMMAND_2 "NOTFOUND")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
