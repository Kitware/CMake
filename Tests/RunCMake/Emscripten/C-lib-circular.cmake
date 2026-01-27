enable_language(C)

add_library(circular1 STATIC circular1.c)
add_library(circular2 STATIC circular2.c)

target_link_libraries(circular1 PRIVATE circular2)
target_link_libraries(circular2 PRIVATE circular1)

add_executable(main_circular main_circular.c)
target_link_libraries(main_circular PRIVATE circular1)
