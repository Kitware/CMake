enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat1 "--start" "--stop")

set(CMAKE_C_LINK_GROUP_USING_feat2_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat2 "--start" "--stop")

add_library(dep1 SHARED empty.c)
add_library(dep2 SHARED empty.c)
add_library(dep3 SHARED empty.c)
target_link_libraries(dep3 PUBLIC "$<LINK_GROUP:feat1,dep1,dep2>")

add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PRIVATE "$<LINK_GROUP:feat2,dep2,dep3>")
