enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--stop")

add_library(dep SHARED empty.c)

# Text concatenated before the genex hides the start of the group.
add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PRIVATE "x$<LINK_GROUP:feat,dep>")

add_library(iface INTERFACE)
target_link_libraries(iface INTERFACE "x$<LINK_GROUP:feat,dep>")
add_library(lib2 SHARED empty.c)
target_link_libraries(lib2 PRIVATE iface)
