add_library(iface INTERFACE)
target_link_libraries(iface PUBLIC PRIVATE INTERFACE)
add_library(imported UNKNOWN IMPORTED)
target_link_libraries(imported PUBLIC PRIVATE INTERFACE)
