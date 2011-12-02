project(override_symbol)

add_library(somelib SHARED someclass.cpp)

set_target_properties(somelib PROPERTIES DEFINE_SYMBOL SOMELIB_MAKEDLL)

generate_export_header(somelib)

add_executable(consumer main.cpp)

target_link_libraries(consumer somelib)
