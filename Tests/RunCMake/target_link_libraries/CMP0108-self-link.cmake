enable_language(C)

add_library(foo SHARED lib.c)
add_library(Bar::foo ALIAS foo)
target_link_libraries(foo PRIVATE Bar::foo)
