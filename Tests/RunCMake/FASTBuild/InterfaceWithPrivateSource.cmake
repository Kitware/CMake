add_library(iface_lib INTERFACE EXCLUDE_FROM_ALL)
target_sources(iface_lib PRIVATE header.hpp)

add_library(static_lib STATIC main.cpp)

target_link_libraries(static_lib PRIVATE iface_lib)
