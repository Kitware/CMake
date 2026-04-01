enable_language(C)

add_library(lib1 STATIC)
target_sources(lib1 INTERFACE FILE_SET a TYPE SOURCES BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES lib1.c)
install(TARGETS lib1 EXPORT a)
