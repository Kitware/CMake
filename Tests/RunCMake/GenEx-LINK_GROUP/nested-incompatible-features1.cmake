enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--end")

add_library(dep1 SHARED empty.c)

add_library(dep2 SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_GROUP:feat,dep1,$<LINK_GROUP:feat,dep2>>")
