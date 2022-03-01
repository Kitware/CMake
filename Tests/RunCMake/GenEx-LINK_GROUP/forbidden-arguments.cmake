enable_language(C)

link_libraries(<LINK_GROUP:feat> foo </LINK_GROUP:feat>)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE <LINK_GROUP:feat> foo </LINK_GROUP:feat>)
