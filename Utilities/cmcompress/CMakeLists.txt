project(CMCompress)

add_library(cmcompress cmcompress.c)

install(FILES Copyright.txt DESTINATION ${CMake_DOC_DEST}/cmcompress)
