
project(CMP0022-WARN)

add_library(foo SHARED empty.cpp)
add_library(bar SHARED empty.cpp)
add_library(bat SHARED empty.cpp)
set_property(TARGET bar PROPERTY INTERFACE_LINK_LIBRARIES foo)
set_property(TARGET bar PROPERTY LINK_INTERFACE_LIBRARIES bat)

add_library(user empty.cpp)
target_link_libraries(user bar)
