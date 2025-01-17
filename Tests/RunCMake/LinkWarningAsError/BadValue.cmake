enable_language(C)

add_executable(main main.c)
set_property(TARGET main PROPERTY LINK_WARNING_AS_ERROR FOO)
