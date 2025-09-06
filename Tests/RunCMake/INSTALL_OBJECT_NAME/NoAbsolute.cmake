add_library(static STATIC lib.c)
set_property(SOURCE lib.c PROPERTY INSTALL_OBJECT_NAME "${CMAKE_BINARY_DIR}/lib.c")
