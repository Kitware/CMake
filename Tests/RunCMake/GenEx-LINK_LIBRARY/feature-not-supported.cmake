enable_language(C)

set(CMAKE_C_LINK_LIBRARY_USING_feat_SUPPORTED FALSE)
set(CMAKE_C_LINK_LIBRARY_USING_feat "<LIBRARY>")

add_library(dep SHARED empty.c)

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE "$<LINK_LIBRARY:feat,dep>")
