set(OUTPUT_NAME "test.tar.lzma")

set(ARCHIVE_FORMAT pax)
set(COMPRESSION_TYPE LZMA)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("5d00008000" LIMIT 5 HEX)
