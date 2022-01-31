
include(incompatible-library-features1.cmake)

set_property(TARGET lib1 PROPERTY LINK_LIBRARY_OVERRIDE "feat1,dep1")
