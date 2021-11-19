
include(incompatible-features1.cmake)

set_property(TARGET lib PROPERTY LINK_LIBRARY_OVERRIDE "feat2,dep1")
