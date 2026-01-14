add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO foo EXPORT foo)
