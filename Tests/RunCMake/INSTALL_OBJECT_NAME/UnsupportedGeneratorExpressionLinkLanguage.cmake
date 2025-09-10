add_library(objlib OBJECT lib.c)
set_property(SOURCE lib.c PROPERTY INSTALL_OBJECT_NAME "$<LINK_LANGUAGE>/objlib_lib.c")
