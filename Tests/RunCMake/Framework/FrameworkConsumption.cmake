
cmake_minimum_required(VERSION 3.22...3.24)
enable_language(C)

# Create framework and ensure header is placed in Headers
set(input_header "${CMAKE_SOURCE_DIR}/Gui.h")
add_library(Gui SHARED Gui.c "${input_header}")
set_target_properties(Gui PROPERTIES
    PUBLIC_HEADER "${input_header}"
    FRAMEWORK TRUE
)

add_executable(app main.c)

target_link_libraries(app PRIVATE Gui)
