add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO test EXPORT foo COMPAT_VERSION 1.0)
