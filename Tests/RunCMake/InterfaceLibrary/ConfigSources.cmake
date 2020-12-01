# Test an interface library added to the build system by a per-config source.
add_library(iface INTERFACE $<$<CONFIG:NotAConfig>:${CMAKE_CURRENT_SOURCE_DIR}/iface.c>)
