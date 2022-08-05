enable_language(C)

set (CMAKE_LINK_LIBRARY_USING_feat "<LIBRARY>")
set (CMAKE_LINK_LIBRARY_USING_feat_SUPPORTED TRUE)

add_library(dep SHARED empty.c)
set_property(TARGET dep PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE "$<LINK_LIBRARY:feat,dep>")

add_library(lib SHARED empty.c)
target_link_libraries(lib PRIVATE dep)
