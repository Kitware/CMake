
project(libshared)

add_library(libshared SHARED libshared.cpp)

generate_export_header(libshared)
