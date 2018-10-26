add_library(iface INTERFACE)
target_compile_options(iface PUBLIC PRIVATE INTERFACE)
# Cannot be called with non-compilable targets.
#add_library(imported UNKNOWN IMPORTED)
#target_compile_options(imported PUBLIC PRIVATE INTERFACE)
