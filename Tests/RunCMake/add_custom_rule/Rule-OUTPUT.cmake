include(check_property.cmake)

unset(TEST_FAILED)

add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 OUTPUT "out1")

add_custom_rule(rule2 OUTPUT out1 out2 COMMAND cmd arg1 arg2)
check_rule_property(rule2 OUTPUT "out1;out2")

add_custom_rule(rule3 OUTPUT out1 out2 COMMAND cmd arg1 arg2 OUTPUT out3 out4)
check_rule_property(rule3 OUTPUT "out1;out2;out3;out4")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
