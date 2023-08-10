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


# Same test but with generation done in custom directories
add_library(Gui2 SHARED Gui.c "${input_header}")
set_target_properties(Gui2 PROPERTIES
    PUBLIC_HEADER "${input_header}"
    FRAMEWORK TRUE
    LIBRARY_OUTPUT_DIRECTORY lib
)

add_executable(app2 main2.c)
set_target_properties(app2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY bin
)

target_link_libraries(app2 PRIVATE Gui2)


# Same test with STATIC consumer
add_library(Consumer STATIC consumer.c)

target_link_libraries(Consumer PRIVATE Gui2)
