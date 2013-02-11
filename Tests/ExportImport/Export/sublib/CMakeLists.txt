
set(CMAKE_BUILD_INTERFACE_INCLUDES ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(subdirlib SHARED subdir.cpp)
generate_export_header(subdirlib)
