set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

add_library(objlib OBJECT lib.c)
set_property(SOURCE lib.c PROPERTY INSTALL_OBJECT_NAME "$<$<BOOL:0>:objlib_lib.c>")
install(TARGETS objlib EXPORT exp OBJECTS DESTINATION "lib/objlib")
install(EXPORT exp DESTINATION lib/cmake/ION FILE ion-config.cmake NAMESPACE ION::)
