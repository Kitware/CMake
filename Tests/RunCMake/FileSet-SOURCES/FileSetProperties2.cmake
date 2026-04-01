enable_language(C)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS OFF)
# ensure no temp file will be used
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")

add_library(lib1 STATIC)
target_sources(lib1 PRIVATE lib1.c)
target_sources(lib1 PUBLIC FILE_SET a TYPE SOURCES FILES lib4.c)

set_property(FILE_SET a TARGET lib1 PROPERTY COMPILE_OPTIONS -DOPT_FS)
set_property(FILE_SET a TARGET lib1 PROPERTY INTERFACE_COMPILE_OPTIONS -DOPT_INTERFACE_FS)

add_executable(main main.c)
target_link_libraries(main PRIVATE lib1)
