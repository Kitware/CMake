PROJECT(CMCompress)

ADD_LIBRARY(cmcompress cmcompress.c)

INSTALL(FILES Copyright.txt DESTINATION ${CMake_DOC_DEST}/cmcompress)
