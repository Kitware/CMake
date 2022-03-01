enable_language(C)

add_library(empty SHARED empty.c)
target_link_directories(empty PRIVATE "$<LINK_GROUP:feat>")
