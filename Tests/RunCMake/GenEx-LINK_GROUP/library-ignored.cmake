enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--end")

add_library(dep OBJECT empty.c)

add_library(lib SHARED empty.c)

add_library(front INTERFACE)
target_link_libraries(front INTERFACE lib)


add_library(lib2 SHARED empty.c)
target_link_libraries(lib2 PRIVATE "$<LINK_GROUP:feat,front,dep>")
