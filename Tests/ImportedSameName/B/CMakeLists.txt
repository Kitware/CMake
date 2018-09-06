add_library(b STATIC b.c)

add_library(sameName INTERFACE IMPORTED)
target_link_libraries(sameName INTERFACE b)

add_library(ifaceB INTERFACE)
target_link_libraries(ifaceB INTERFACE sameName)
