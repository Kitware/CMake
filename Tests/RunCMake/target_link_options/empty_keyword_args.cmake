add_library(iface INTERFACE)
target_link_options(iface PUBLIC PRIVATE INTERFACE)
# Cannot be called with non-compilable targets.
#add_library(imported UNKNOWN IMPORTED)
#target_link_options(imported PUBLIC PRIVATE INTERFACE)
