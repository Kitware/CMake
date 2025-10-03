add_library(foo INTERFACE SYMBOLIC)

install(TARGETS foo EXPORT foo)
install(PACKAGE_INFO foo EXPORT foo DESTINATION .)
