add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO test EXPORT foo APPENDIX test DEFAULT_TARGETS foo)
