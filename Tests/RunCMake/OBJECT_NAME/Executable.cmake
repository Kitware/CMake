set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

add_executable(exe main.c lib.c)
set_property(SOURCE main.c PROPERTY OBJECT_NAME "exe_main.c")
