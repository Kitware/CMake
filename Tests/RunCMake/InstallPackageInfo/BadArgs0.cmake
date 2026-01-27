install(PACKAGE_INFO)
install(PACKAGE_INFO test)
install(PACKAGE_INFO test EXPORT)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO EXPORT foo)
