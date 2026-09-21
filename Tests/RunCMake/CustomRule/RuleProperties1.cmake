
add_custom_rule(foo OUTPUT out1 COMMAND cmd arg1 arg2)


set_property(RULE foo PROPERTY OUTPUT_FILE_SET wrong)
set_property(RULE foo PROPERTY OUTPUT_FILE_SET name wrong_type)


set_property(RULE foo PROPERTY GLOBAL false)
