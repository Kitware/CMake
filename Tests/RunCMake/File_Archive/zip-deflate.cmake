set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT zip)
set(COMPRESSION_TYPE Deflate)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
