set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

add_library(sharedlib SHARED lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "sharedlib_lib.c")
