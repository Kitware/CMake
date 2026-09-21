
add_custom_rule(foo OUTPUT out1 COMMAND cmd arg1 arg2)


set_property(RULE foo PROPERTY OUTPUT_FILE_SET bad_<PATTERN>_<RULE>_<TARGET>_<FILE_SET> SOURCES)

add_library(bar)
target_sources(bar PRIVATE FILE_SET fs TYPE foo FILES file1.c)
