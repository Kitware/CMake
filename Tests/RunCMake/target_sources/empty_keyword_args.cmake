add_library(iface INTERFACE)
target_sources(iface PUBLIC PRIVATE INTERFACE)
# Cannot be called with non-compilable targets.
#add_library(imported UNKNOWN IMPORTED)
#target_sources(imported PUBLIC PRIVATE INTERFACE)
