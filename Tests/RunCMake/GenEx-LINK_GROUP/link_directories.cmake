enable_language(C)

link_directories("$<LINK_GROUP:feat>")

add_library(empty SHARED empty.c)
