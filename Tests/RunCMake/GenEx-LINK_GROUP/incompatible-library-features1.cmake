enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat1 "--start" "--stop")

set(CMAKE_C_LINK_LIBRARY_USING_feat1_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat1 "--libflag1<LIBRARY>")

set(CMAKE_C_LINK_LIBRARY_USING_feat2_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat2 "--libflag2<LIBRARY>")

add_library(dep1 SHARED empty.c)
add_library(dep2 SHARED empty.c)
target_link_libraries(dep2 PUBLIC "$<LINK_LIBRARY:feat1,dep1>")

add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PRIVATE "$<LINK_GROUP:feat1,$<LINK_LIBRARY:feat2,dep2>,dep1>")
