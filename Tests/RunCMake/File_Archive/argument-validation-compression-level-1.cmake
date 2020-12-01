set(OUTPUT_NAME "test.tar.gz")

set(ARCHIVE_FORMAT gnutar)
set(COMPRESSION_TYPE GZip)

include(${CMAKE_CURRENT_LIST_DIR}/compression-level.cmake)

check_compression_level("100")
