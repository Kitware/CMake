enable_language(C)

add_library(A INTERFACE IMPORTED)
add_library(B INTERFACE IMPORTED)
add_library(C INTERFACE IMPORTED)
set_property(TARGET A PROPERTY IMPORTED_LIBNAME A)
set_property(TARGET B PROPERTY IMPORTED_LIBNAME B)
set_property(TARGET C PROPERTY IMPORTED_LIBNAME C)

add_executable(main Duplicate.c)
target_link_libraries(main PRIVATE -Flag1 A -Flag1 B -Flag2 C -Flag2 A -Flag2 B -Flag3 C)
set_property(TARGET main PROPERTY LINK_DEPENDS_DEBUG_MODE 1) # undocumented
