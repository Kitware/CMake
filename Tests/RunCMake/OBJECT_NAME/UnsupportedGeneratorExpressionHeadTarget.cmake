add_library(objlib OBJECT lib.c)
set_property(SOURCE lib.c PROPERTY OBJECT_NAME "$<TARGET_PROPERTY:foo>/objlib_lib.c")
