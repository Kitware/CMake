
enable_language(C)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gen.h"
                   COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/gen.c.in"
                                                                   "${CMAKE_CURRENT_BINARY_DIR}/gen.h"
                   DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gen.c.in")

add_library(lib1 STATIC lib1.c)
target_sources(lib1 PRIVATE FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
                                             FILES "${CMAKE_CURRENT_BINARY_DIR}/gen.h")

set_property(FILE_SET HEADERS TARGET lib1 PROPERTY INDEPENDENT_FILES ON)
