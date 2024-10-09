enable_language(C)

add_executable(LargeELF LargeELF.c)
set_property(TARGET LargeELF PROPERTY INSTALL_RPATH "/test")
install(TARGETS LargeELF)
