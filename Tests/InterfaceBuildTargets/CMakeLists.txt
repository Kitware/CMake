project(InterfaceBuildTargets)

add_library(testlib testlib.cxx)
set_property(TARGET testlib PROPERTY PREFIX "")
set_property(TARGET testlib PROPERTY SUFFIX "")

add_library(iface INTERFACE)
target_link_libraries(iface INTERFACE testlib)
