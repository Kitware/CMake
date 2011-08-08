
project(libstatic)

# Show that the export header has no effect on a static library.

add_library(libstatic STATIC libstatic.cpp)

generate_export_header(libstatic)
