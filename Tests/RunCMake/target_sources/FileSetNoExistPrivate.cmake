enable_language(C)

add_library(lib1 STATIC empty.c)
set_property(TARGET lib1 PROPERTY HEADER_SETS "a")

# Error happens at configure-time, so this doesn't help.
target_sources(lib1 PRIVATE FILE_SET a TYPE HEADERS)
