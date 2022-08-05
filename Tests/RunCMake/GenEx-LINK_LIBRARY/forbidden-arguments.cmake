enable_language(C)

link_libraries(<LINK_LIBRARY:feat> foo </LINK_LIBRARY:feat>)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE <LINK_LIBRARY:feat> foo </LINK_LIBRARY:feat>)
