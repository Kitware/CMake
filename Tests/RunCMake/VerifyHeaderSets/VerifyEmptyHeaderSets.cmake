enable_language(C)

# Target with VERIFY_INTERFACE_HEADER_SETS ON but no interface file sets
add_library(empty_iface STATIC lib.c)
set_property(TARGET empty_iface PROPERTY VERIFY_INTERFACE_HEADER_SETS ON)

# Target with VERIFY_PRIVATE_HEADER_SETS ON but no private file sets
add_library(empty_priv STATIC lib.c)
set_property(TARGET empty_priv PROPERTY VERIFY_PRIVATE_HEADER_SETS ON)
