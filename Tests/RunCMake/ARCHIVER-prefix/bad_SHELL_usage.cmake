
enable_language(C)

add_library(example STATIC lib.c)
set_property(TARGET example PROPERTY STATIC_LIBRARY_OPTIONS "ARCHIVER:-foo,SHELL:-bar")
