# Prevent duplicate errors on some platforms.
set(CMAKE_IMPORT_LIBRARY_SUFFIX "placeholder")

add_library(unknown_lib UNKNOWN IMPORTED)
add_library(static_lib STATIC IMPORTED)
add_library(shared_lib SHARED IMPORTED)
add_library(interface_lib INTERFACE IMPORTED)

add_library(module MODULE module.cpp)
target_link_libraries(module unknown_lib static_lib shared_lib interface_lib)
