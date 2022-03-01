
include(incompatible-library-features2.cmake)

set_property(TARGET lib1 PROPERTY LINK_LIBRARY_OVERRIDE_dep1 "feat1")
