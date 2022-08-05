enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat1 "--start" "--end")

set(CMAKE_C_LINK_GROUP_USING_feat2_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat2 "--start" "--end")

add_library(dep1 SHARED empty.c)

add_library(dep2 SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_GROUP:feat1,dep1,$<LINK_GROUP:feat2,dep2>>")
