
function(configurator1 rule target fileset patterns)
  set_property(RULE ${rule} PROPERTY COMMENT "bar")
endfunction()

add_custom_rule(foo1 OUTPUT out1 COMMAND cmd arg1 arg2 CONFIGURATOR FOR_FILE_SET configurator1)

function(configurator2 rule target fileset patterns)
  set_property(RULE ${rule} PROPERTY CUSTOM "bar")
endfunction()

add_custom_rule(foo2 OUTPUT out1 COMMAND cmd arg1 arg2 CONFIGURATOR FOR_FILE_SET configurator2)

add_library(foo)
target_sources(foo PRIVATE FILE_SET fs1 TYPE foo1 FILES file1.c)
target_sources(foo PRIVATE FILE_SET fs2 TYPE foo2 FILES file2.c)
