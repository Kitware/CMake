add_library(iface INTERFACE)
target_include_directories(iface PUBLIC PRIVATE INTERFACE)
# Cannot be called with non-compilable targets.
#add_library(imported UNKNOWN IMPORTED)
#target_include_directories(imported PUBLIC PRIVATE INTERFACE)
