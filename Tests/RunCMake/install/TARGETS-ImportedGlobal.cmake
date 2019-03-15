add_library(imported_global STATIC IMPORTED GLOBAL)
set_property(TARGET imported_global PROPERTY IMPORTED_LOCATION /does_not_exist)
install(TARGETS imported_global DESTINATION bin)
