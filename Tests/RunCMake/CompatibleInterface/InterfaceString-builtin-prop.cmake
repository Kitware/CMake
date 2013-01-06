
add_library(foo UNKNOWN IMPORTED)
add_library(bar UNKNOWN IMPORTED)

set_property(TARGET foo APPEND PROPERTY COMPATIBLE_INTERFACE_STRING INCLUDE_DIRECTORIES)
set_property(TARGET foo PROPERTY INTERFACE_INCLUDE_DIRECTORIES foo_inc)
set_property(TARGET bar PROPERTY INTERFACE_INCLUDE_DIRECTORIES bar_inc)

add_executable(user main.cpp)
set_property(TARGET user PROPERTY INCLUDE_DIRECTORIES bar_inc)
target_link_libraries(user foo bar)
