add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Try exporting a non-conforming version.
install(PACKAGE_INFO foo EXPORT foo VERSION "1.2.3rc1")
