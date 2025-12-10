cmake_policy(SET CMP0208 OLD)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo)

export(EXPORT foo FILE)
