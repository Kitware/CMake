add_library(objlib OBJECT lib.c)
set_property(SOURCE lib.c PROPERTY INSTALL_OBJECT_NAME "$<TARGET_PROPERTY:foo>/objlib_lib.c")
