
enable_language(C)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/custom.c"
                   COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib1.c"
                                                      "${CMAKE_CURRENT_BINARY_DIR}/custom.c")

add_library(lib1 STATIC)
target_sources(lib1 PRIVATE FILE_SET SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                             FILES "${CMAKE_CURRENT_BINARY_DIR}/custom.c")
