
add_library(foo empty.cpp)
add_library(iface INTERFACE)
add_dependencies(iface foo)
