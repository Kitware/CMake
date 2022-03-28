enable_language(C)

# Language specific definition takes precedence over more generic one
set(CMAKE_C_LINK_GROUP_USING_feat "-BEFORE" "-AFTER")
set(CMAKE_C_LINK_GROUP_USING_feat_SUPPORTED FALSE)
set(CMAKE_LINK_GROUP_USING_feat "-BEFORE" "-AFTER")
set(CMAKE_LINK_GROUP_USING_feat_SUPPORTED TRUE)

add_library(dep SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_GROUP:feat,dep>")
