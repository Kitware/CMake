add_library(iface INTERFACE)
target_include_directories(iface SYSTEM INTERFACE "$<INSTALL_INTERFACE:include>")
install(TARGETS iface EXPORT foo)
install(EXPORT foo DESTINATION lib/cmake/foo)
