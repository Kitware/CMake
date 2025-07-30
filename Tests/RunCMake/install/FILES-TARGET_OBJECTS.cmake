set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)
add_library(objs OBJECT obj1.c obj2.c)
install(FILES $<TARGET_OBJECTS:objs> DESTINATION objs)
