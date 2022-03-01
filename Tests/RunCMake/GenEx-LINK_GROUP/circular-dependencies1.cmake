enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--stop")

add_library(dep1.1 SHARED empty.c)
add_library(dep1.2 SHARED empty.c)

add_library(dep2.1 SHARED empty.c)
add_library(dep2.2 SHARED empty.c)

target_link_libraries(dep1.1 PUBLIC dep2.1)
target_link_libraries(dep2.2 PUBLIC dep1.2)

add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PRIVATE "$<LINK_GROUP:feat,dep1.1,dep1.2>"
                                   "$<LINK_GROUP:feat,dep2.1,dep2.2>")
