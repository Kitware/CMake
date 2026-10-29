enable_language(C)

# lib1 -> lib2 -> static -> iface -> $<TARGET_RUNTIME_DLLS:lib1>
add_library(lib1 SHARED lib1.c)
add_library(lib2 SHARED lib2.c)
add_library(static STATIC static.c)
add_library(iface INTERFACE)
set_target_properties(iface PROPERTIES
  COMPATIBLE_INTERFACE_BOOL FOO
  INTERFACE_FOO ON
  )
target_link_libraries(iface INTERFACE $<TARGET_RUNTIME_DLLS:lib1>)
target_link_libraries(static PUBLIC iface)
target_link_libraries(lib2 PUBLIC static)
target_link_libraries(lib1 PRIVATE lib2)

# Evaluate the link interface before the sources of lib1 are computed.
file(GENERATE OUTPUT "out-$<TARGET_PROPERTY:lib1,FOO>.txt" CONTENT "")
