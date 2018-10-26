add_library(iface INTERFACE)
target_compile_definitions(iface PUBLIC PRIVATE INTERFACE)
# Cannot be called with non-compilable targets.
#add_library(imported UNKNOWN IMPORTED)
#target_compile_definitions(imported PUBLIC PRIVATE INTERFACE)
