set(COMPRESS_DIR ${CMAKE_CURRENT_BINARY_DIR}/compress_dir)
file(WRITE ${COMPRESS_DIR}/f1.txt "f1")

file(ARCHIVE_CREATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/out.tar
  FORMAT gnutar
  PATHS ${COMPRESS_DIR}
  PATTERNS_EXCLUDE "*.o" "")    # empty pattern is rejected
