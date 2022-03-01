enable_language(C)

set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_feat "-start" "<LIBRARY>" "-stop")

add_library(dep SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_GROUP:feat,dep>")
