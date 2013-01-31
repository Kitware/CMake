
set(CMAKE_BUILD_INTERFACE_INCLUDES ON)

add_library(subdirlib SHARED subdirlib.cpp)
generate_export_header(subdirlib)
