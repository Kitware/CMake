include(check_property.cmake)

unset(TEST_FAILED)

function(fileset1_configurator)
endfunction()
function(fileset2_configurator)
endfunction()
function(fileset3_configurator)
endfunction()

function(source1_configurator)
endfunction()
function(source2_configurator)
endfunction()
function(source3_configurator)
endfunction()

add_custom_rule(rule1 OUTPUT out1 COMMAND cmd arg1 arg2)
check_rule_property(rule1 FILE_SET_CONFIGURATORS "")
check_rule_property(rule1 SOURCE_CONFIGURATORS "")

add_custom_rule(rule2 OUTPUT out1 COMMAND cmd arg1 arg2 CONFIGURATOR FOR_FILE_SET fileset1_configurator)
check_rule_property(rule2 FILE_SET_CONFIGURATORS "fileset1_configurator")
check_rule_property(rule2 SOURCE_CONFIGURATORS "")

add_custom_rule(rule3 OUTPUT out1 COMMAND cmd arg1 arg2 CONFIGURATOR FOR_SOURCE source1_configurator)
check_rule_property(rule3 FILE_SET_CONFIGURATORS "")
check_rule_property(rule3 SOURCE_CONFIGURATORS "source1_configurator")

add_custom_rule(rule4 OUTPUT out1 COMMAND cmd arg1 arg2 CONFIGURATOR FOR_FILE_SET fileset1_configurator
                                                                     FOR_SOURCE source1_configurator)
check_rule_property(rule4 FILE_SET_CONFIGURATORS "fileset1_configurator")
check_rule_property(rule4 SOURCE_CONFIGURATORS "source1_configurator")


add_custom_rule(rule5 FROM_RULE rule4)
check_rule_property(rule5 FILE_SET_CONFIGURATORS "fileset1_configurator")
check_rule_property(rule5 SOURCE_CONFIGURATORS "source1_configurator")

add_custom_rule(rule6 FROM_RULE rule4 CONFIGURATOR FOR_FILE_SET fileset2_configurator
                                                   FOR_SOURCE source2_configurator)
check_rule_property(rule6 FILE_SET_CONFIGURATORS "fileset2_configurator")
check_rule_property(rule6 SOURCE_CONFIGURATORS "source2_configurator")

add_custom_rule(rule7 FROM_RULE rule4 CONFIGURATOR FOR_FILE_SET fileset2_configurator OVERRIDE
                                                   FOR_SOURCE source2_configurator OVERRIDE)
check_rule_property(rule7 FILE_SET_CONFIGURATORS "fileset2_configurator")
check_rule_property(rule7 SOURCE_CONFIGURATORS "source2_configurator")

add_custom_rule(rule8 FROM_RULE rule4 CONFIGURATOR FOR_FILE_SET fileset2_configurator CHAIN
                                                   FOR_SOURCE source2_configurator CHAIN)
check_rule_property(rule8 FILE_SET_CONFIGURATORS "fileset1_configurator;fileset2_configurator")
check_rule_property(rule8 SOURCE_CONFIGURATORS "source1_configurator;source2_configurator")

add_custom_rule(rule9 FROM_RULE rule8 CONFIGURATOR FOR_FILE_SET fileset3_configurator CHAIN
                                                   FOR_SOURCE source3_configurator CHAIN)
check_rule_property(rule9 FILE_SET_CONFIGURATORS "fileset1_configurator;fileset2_configurator;fileset3_configurator")
check_rule_property(rule9 SOURCE_CONFIGURATORS "source1_configurator;source2_configurator;source3_configurator")

if(TEST_FAILED)
  message(FATAL_ERROR "${TEST_FAILED}")
endif()
