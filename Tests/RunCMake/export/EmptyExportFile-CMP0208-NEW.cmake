cmake_policy(SET CMP0208 NEW)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo)

export(EXPORT foo FILE)
