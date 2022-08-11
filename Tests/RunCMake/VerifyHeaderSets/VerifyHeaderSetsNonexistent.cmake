enable_language(C)

add_library(nonexistent STATIC lib.c)
target_sources(nonexistent INTERFACE FILE_SET a TYPE HEADERS FILES a.h)
set_property(TARGET nonexistent PROPERTY INTERFACE_HEADER_SETS_TO_VERIFY "a;c;b")
