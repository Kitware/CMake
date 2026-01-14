export(PACKAGE_INFO)
export(PACKAGE_INFO test)
export(PACKAGE_INFO test EXPORT)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO EXPORT foo)
