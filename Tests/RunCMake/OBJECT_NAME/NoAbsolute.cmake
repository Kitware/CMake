add_library(static STATIC lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "${CMAKE_BINARY_DIR}/lib.c")
