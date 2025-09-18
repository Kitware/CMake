set(CMAKE_INTERMEDIATE_DIR_STRATEGY SHORT CACHE STRING "" FORCE)
enable_language(C)

add_library(objlib OBJECT subdir/obj.c)
install(TARGETS objlib OBJECTS DESTINATION lib/objlib)
