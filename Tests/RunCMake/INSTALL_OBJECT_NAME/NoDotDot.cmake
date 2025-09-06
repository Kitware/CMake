add_library(static STATIC lib.c)
set_property(SOURCE lib.c PROPERTY INSTALL_OBJECT_NAME "subdir/../sibling/lib.c")
