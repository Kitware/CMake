enable_language(C)

add_library(lib1 STATIC empty.c)
install(TARGETS lib1 EXPORT a)
target_sources(lib1 INTERFACE FILE_SET a TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES h1.h)
install(EXPORT a DESTINATION lib/cmake/test)
