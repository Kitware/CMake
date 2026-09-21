include(check_property.cmake)

unset(TEST_FAILED)

add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 DEPFILE "")

add_custom_rule(rule2 OUTPUT out1 COMMAND cmd arg1 arg2 DEPFILE depfile)
check_rule_property(rule2 DEPFILE "depfile")

add_custom_rule(rule3 OUTPUT out1 out2 DEPFILE depfile COMMAND cmd arg1 arg2)
check_rule_property(rule3 DEPFILE "depfile")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
