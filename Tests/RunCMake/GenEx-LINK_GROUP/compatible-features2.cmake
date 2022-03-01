enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--stop")

add_library(dep1 SHARED empty.c)
add_library(dep2 SHARED empty.c)

add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PUBLIC "$<LINK_GROUP:feat,dep1,dep2>")

add_library(lib2 SHARED empty.c)
target_link_libraries(lib2 PRIVATE "$<LINK_GROUP:feat,dep2,lib1>")
