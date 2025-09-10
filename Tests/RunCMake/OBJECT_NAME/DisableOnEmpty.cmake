set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

add_library(objlib OBJECT lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "$<$<BOOL:0>:objlib_lib.c>")
install(TARGETS objlib EXPORT exp OBJECTS DESTINATION "lib/objlib")
install(EXPORT exp DESTINATION lib/cmake/ON FILE on-config.cmake NAMESPACE ON::)
