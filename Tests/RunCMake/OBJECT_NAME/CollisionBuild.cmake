set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

add_library(objlib OBJECT lib.c lib2.c)
set_property(SOURCE lib.c lib2.c PROPERTY OBJECT_NAME "objlib_lib.c")

add_executable(usefuncs test.c)
target_link_libraries(usefuncs PRIVATE objlib)
