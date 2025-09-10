add_library(static STATIC lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "subdir/../sibling/lib.c")
