add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Try exporting a non-conforming version.
export(EXPORT foo PACKAGE_INFO foo VERSION "1.2.3rc1")
