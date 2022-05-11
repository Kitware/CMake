enable_language(C)

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET a TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES h1.h)
install(TARGETS lib1 EXPORT a)

add_library(lib2 STATIC empty.c)
target_sources(lib2 INTERFACE FILE_SET a TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES h1.h)
install(TARGETS lib2)
