add_library(iface INTERFACE)

# SYSTEM is a recognized keyword for the base class used to implement the
# command. Verify that we don't allow it.
target_compile_options(iface SYSTEM PRIVATE)
