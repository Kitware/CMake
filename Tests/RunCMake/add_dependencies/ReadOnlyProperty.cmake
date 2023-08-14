enable_language(C)

add_library(a a.c)

set_property(TARGET a PROPERTY MANUALLY_ADDED_DEPENDENCIES DEPENDENCIES foo)
