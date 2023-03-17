
enable_language(C)

set_property(SOURCE foo.c PROPERTY COMPILE_DEFINITIONS -DDEF0 "$<1:-DDEF1>")

add_library(lib1 foo.c)
set_property(TARGET lib1 PROPERTY COMPILE_DEFINITIONS -DDEF2 "$<1:-DDEF3>")
set_property(TARGET lib1 PROPERTY INTERFACE_COMPILE_DEFINITIONS -DDEF4 "$<1:-DDEF5>")

add_library(lib2 foo.c)
target_link_libraries(lib2 PRIVATE lib1)
