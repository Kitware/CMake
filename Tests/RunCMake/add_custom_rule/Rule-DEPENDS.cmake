include(check_property.cmake)

unset(TEST_FAILED)

add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 DEPENDS "")

add_custom_rule(rule2 OUTPUT out1 COMMAND cmd arg1 arg2 DEPENDS dep1 dep2)
check_rule_property(rule2 DEPENDS "dep1;dep2")

add_custom_rule(rule3 OUTPUT out1 out2 DEPENDS dep1 dep2 COMMAND cmd arg1 arg2)
check_rule_property(rule3 DEPENDS "dep1;dep2")

add_custom_rule(rule4 OUTPUT out1 out2 DEPENDS dep1 dep2 COMMAND cmd arg1 arg2 DEPENDS dep3 dep4)
check_rule_property(rule4 DEPENDS "dep1;dep2;dep3;dep4")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
