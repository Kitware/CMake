
enable_language(C)

add_custom_rule(foo OUTPUT out
  COMMAND cmd <COMPILE_OPTIONS>)

set_property(RULE foo PROPERTY COMPILE_OPTIONS "$<RULE_PROPERTY:<RULE>,COMPILE_OPTIONS>")

add_library(foo foo.c)

target_sources(foo PRIVATE FILE_SET fs TYPE foo FILES foo.txt)
