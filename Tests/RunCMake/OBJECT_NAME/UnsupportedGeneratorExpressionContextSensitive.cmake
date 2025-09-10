add_library(objlib OBJECT lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "$<CONFIG>/objlib_lib.c")
