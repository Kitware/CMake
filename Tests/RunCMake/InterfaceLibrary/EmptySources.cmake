# Test an interface library added to the build system by empty SOURCES.
add_library(iface INTERFACE)
set_property(TARGET iface PROPERTY SOURCES "")

# ...but not added by unset SOURCES.
add_library(iface2 INTERFACE)
set_property(TARGET iface2 PROPERTY SOURCES "")
set_property(TARGET iface2 PROPERTY SOURCES)
