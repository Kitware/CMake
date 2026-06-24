include(check_property.cmake)

unset(TEST_FAILED)


add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 GLOBAL "0")

add_custom_rule(rule2 OUTPUT out1 COMMAND cmd arg1 arg2 GLOBAL)
check_rule_property(rule2 GLOBAL "1")


add_custom_rule(rule3 FROM_RULE rule1)
check_rule_property(rule3 GLOBAL "0")

add_custom_rule(rule4 FROM_RULE rule1 GLOBAL)
check_rule_property(rule4 GLOBAL "1")

add_custom_rule(rule5 FROM_RULE rule2)
check_rule_property(rule5 GLOBAL "0")

add_custom_rule(rule6 FROM_RULE rule2 GLOBAL)
check_rule_property(rule6 GLOBAL "1")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
