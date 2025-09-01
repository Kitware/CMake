add_library(foo INTERFACE SYMBOLIC)
install(TARGETS foo EXPORT foo)
install(EXPORT foo DESTINATION . FILE foo.cmake)
