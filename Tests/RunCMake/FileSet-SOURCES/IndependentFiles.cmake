
enable_language(C)

set(CACHE{CMAKE_INTERMEDIATE_DIR_STRATEGY} TYPE STRING FORCE VALUE FULL)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gen.c"
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/gen.c.in"
                                                                   "${CMAKE_CURRENT_BINARY_DIR}/gen.c"
                   DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gen.c.in")

add_library(lib1 STATIC)
target_sources(lib1 PRIVATE independent.c
                    PRIVATE FILE_SET SOURCES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                             FILES "${CMAKE_CURRENT_BINARY_DIR}/gen.c")

set_property(FILE_SET SOURCES TARGET lib1 PROPERTY INDEPENDENT_FILES ON)
