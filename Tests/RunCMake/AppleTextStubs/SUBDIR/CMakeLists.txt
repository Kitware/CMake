
add_library(foo2 SHARED ../foo.c)
set_property(TARGET foo2 PROPERTY ENABLE_EXPORTS TRUE)

install(TARGETS foo2 DESTINATION "${CMAKE_BINARY_DIR}/INSTALL")
