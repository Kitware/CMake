enable_language(C)

add_library(Unset STATIC empty.c)
add_library(ToBeSet STATIC expected_file.c)

# Only one target with EXPORT_COMPILE_COMMANDS property.
set_property(TARGET ToBeSet PROPERTY EXPORT_COMPILE_COMMANDS TRUE)
