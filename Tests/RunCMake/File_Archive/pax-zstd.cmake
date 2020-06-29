set(OUTPUT_NAME "test.tar.zstd")

set(ARCHIVE_FORMAT pax)
set(COMPRESSION_TYPE Zstd)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("28b52ffd0058" LIMIT 6 HEX)
