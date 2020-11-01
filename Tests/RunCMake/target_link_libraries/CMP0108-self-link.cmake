
cmake_policy (SET CMP0038 NEW)
cmake_policy (SET CMP0042 NEW)

enable_language(C)

add_library(foo SHARED lib.c)
add_library(Bar::foo ALIAS foo)
target_link_libraries(foo PRIVATE Bar::foo)
