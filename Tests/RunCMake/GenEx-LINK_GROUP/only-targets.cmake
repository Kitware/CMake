enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "--start" "--end")

set(CMAKE_LINK_LIBRARIES_ONLY_TARGETS 1)

add_library(dep1 SHARED empty.c)

add_library(lib1 SHARED empty.c)
# accepted
target_link_libraries(lib1 PRIVATE "$<LINK_GROUP:feat,dep1>")

add_library(lib2 SHARED empty.c)
# invalid
target_link_libraries(lib2 PRIVATE "$<LINK_GROUP:feat,external>")
