enable_language(C)

link_directories("$<LINK_LIBRARY:feat>")

add_library(empty SHARED empty.c)
