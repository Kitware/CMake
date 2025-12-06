set(OUTPUT_NAME "test.tar.gz")

set(ARCHIVE_FORMAT gnutar)
set(COMPRESSION_TYPE Deflate)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("1f8b" LIMIT 2 HEX)
