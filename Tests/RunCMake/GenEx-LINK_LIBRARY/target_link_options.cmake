enable_language(C)

add_library(empty SHARED empty.c)
target_link_options(empty PRIVATE "$<LINK_LIBRARY:FEAT>")
