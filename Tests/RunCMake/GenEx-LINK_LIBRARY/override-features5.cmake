
include(incompatible-features1.cmake)


set_property(TARGET lib PROPERTY LINK_LIBRARY_OVERRIDE "feat1,dep1")
# next property will be ignored because no feature is specified
set_property(TARGET lib PROPERTY LINK_LIBRARY_OVERRIDE_dep1)
