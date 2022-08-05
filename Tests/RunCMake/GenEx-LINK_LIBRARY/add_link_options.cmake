enable_language(C)

add_link_options("$<LINK_LIBRARY:feat>")

add_library(empty SHARED empty.c)
