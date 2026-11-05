
enable_language(C)

add_custom_rule(simple OUTPUT <OUTPUT_FILE>
  COMMAND cmd)

add_library(foo STATIC)

target_sources(foo PRIVATE FILE_SET fs TYPE simple FILES file.i)
set_property(FILE_SET fs TARGET foo PROPERTY RULE_PATTERNS "OUTPUT_FILE=<OUTPUT_FILE>")
