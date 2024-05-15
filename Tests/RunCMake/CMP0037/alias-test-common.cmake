enable_testing()
add_library(iface INTERFACE)
add_library(test ALIAS iface)
