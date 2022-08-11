enable_language(C)

add_link_options("$<LINK_GROUP:feat>")

add_library(empty SHARED empty.c)
