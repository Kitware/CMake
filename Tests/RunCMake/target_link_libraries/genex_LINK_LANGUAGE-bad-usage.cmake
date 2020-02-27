enable_language(C)

add_library(simple SHARED empty.c)
target_link_libraries(simple PRIVATE lib$<LINK_LANGUAGE>)
