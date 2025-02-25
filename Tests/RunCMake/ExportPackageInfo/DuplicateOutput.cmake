add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
export(EXPORT foo PACKAGE_INFO foo)
export(EXPORT foo PACKAGE_INFO foo)
