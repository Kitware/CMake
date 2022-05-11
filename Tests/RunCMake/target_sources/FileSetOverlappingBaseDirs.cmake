enable_language(C)

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET a TYPE HEADERS BASE_DIRS $<1:${CMAKE_CURRENT_SOURCE_DIR}/.$<SEMICOLON>${CMAKE_CURRENT_SOURCE_DIR}/dir3> FILES h1.h)
