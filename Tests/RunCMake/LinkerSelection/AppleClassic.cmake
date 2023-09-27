
enable_language(C)

set(CMAKE_LINKER_TYPE APPLE_CLASSIC)

add_executable(main main.c)
target_link_libraries(main PRIVATE m m)
