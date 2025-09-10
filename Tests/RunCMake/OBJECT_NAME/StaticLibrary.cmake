set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

add_library(staticlib STATIC lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "staticlib_lib.c")
