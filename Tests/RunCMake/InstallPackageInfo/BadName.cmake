add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO "%foo" EXPORT foo)
