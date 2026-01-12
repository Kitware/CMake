add_library(foo INTERFACE SYMBOLIC)

install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO foo EXPORT foo)
