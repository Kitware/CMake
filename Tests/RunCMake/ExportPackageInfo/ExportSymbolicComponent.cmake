add_library(foo INTERFACE SYMBOLIC)

install(TARGETS foo EXPORT foo DESTINATION .)
export(EXPORT foo PACKAGE_INFO foo)
