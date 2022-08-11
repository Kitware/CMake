
include(incompatible-features1.cmake)


set(CMAKE_C_LINK_LIBRARY_USING_feat3_SUPPORTED TRUE)
set(CMAKE_C_LINK_LIBRARY_USING_feat3 "<LIBRARY>")

set_property(TARGET lib PROPERTY LINK_LIBRARY_OVERRIDE "feat3,dep1")
set_property(TARGET lib PROPERTY LINK_LIBRARY_OVERRIDE_dep1 feat1)
