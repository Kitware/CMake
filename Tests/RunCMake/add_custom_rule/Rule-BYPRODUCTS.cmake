include(check_property.cmake)

unset(TEST_FAILED)

add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 BYPRODUCTS "")

add_custom_rule(rule2 OUTPUT out1 COMMAND cmd arg1 arg2 BYPRODUCTS bp1 bp2)
check_rule_property(rule2 BYPRODUCTS "bp1;bp2")

add_custom_rule(rule3 BYPRODUCTS bp1 bp2 OUTPUT out1 out2 COMMAND cmd arg1 arg2)
check_rule_property(rule3 BYPRODUCTS "bp1;bp2")

add_custom_rule(rule4 OUTPUT out1 out2 BYPRODUCTS bp1 bp2 COMMAND cmd arg1 arg2 BYPRODUCTS bp3 bp4)
check_rule_property(rule4 BYPRODUCTS "bp1;bp2;bp3;bp4")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
