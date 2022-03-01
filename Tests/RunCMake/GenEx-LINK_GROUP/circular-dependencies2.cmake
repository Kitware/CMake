enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--stop")

add_library(base1 SHARED empty.c)
add_library(base2 SHARED empty.c)
add_library(base3 SHARED empty.c)
add_library(base4 SHARED empty.c)

target_link_libraries(base1 PUBLIC base3)
target_link_libraries(base4 PUBLIC base2)

add_library(lib1 SHARED empty.c)
target_link_libraries(lib1 PUBLIC "$<LINK_GROUP:feat,base1,base2>")

add_library(lib2 SHARED empty.c)
target_link_libraries(lib2 PRIVATE "$<LINK_GROUP:feat,base3,base4>" lib1)
