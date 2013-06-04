
project(CMP0022-WARN-tll)

add_library(foo SHARED empty.cpp)
add_library(bar SHARED empty.cpp)
add_library(bat SHARED empty.cpp)
target_link_libraries(bar LINK_PUBLIC foo)
set_property(TARGET bar PROPERTY LINK_INTERFACE_LIBRARIES bat)

add_library(user SHARED empty.cpp)
target_link_libraries(user bar)
