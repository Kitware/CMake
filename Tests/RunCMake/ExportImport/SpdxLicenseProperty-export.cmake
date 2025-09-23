add_library(foo INTERFACE)
set_property(TARGET foo PROPERTY SPDX_LICENSE "BSD-3-Clause")

install(TARGETS foo EXPORT foo)
install(EXPORT foo DESTINATION lib/cmake/foo)
install(FILES foo-config.cmake.in RENAME foo-config.cmake DESTINATION lib/cmake/foo)
