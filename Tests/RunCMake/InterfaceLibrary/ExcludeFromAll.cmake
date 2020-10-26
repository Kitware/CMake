# Test an interface library with a custom command, but excluded from all.
add_custom_command(OUTPUT iface.txt COMMAND ${CMAKE_COMMAND} -E touch iface.txt)
add_library(iface INTERFACE EXCLUDE_FROM_ALL iface.txt)

# Test that EXCLUDE_FROM_ALL is allowed even if the interface library has
# no sources, and does not cause it to appear in the build system.
add_library(iface2 INTERFACE EXCLUDE_FROM_ALL)
