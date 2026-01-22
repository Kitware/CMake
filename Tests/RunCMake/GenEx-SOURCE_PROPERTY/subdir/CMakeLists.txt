
set_property(SOURCE ../foo.c PROPERTY FOO BAR)

add_library(foo STATIC ../foo.c)
