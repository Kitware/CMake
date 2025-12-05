add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo)

export(EXPORT foo FILE)
