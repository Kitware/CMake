set(OUTPUT_NAME "test.tar.bz2")

set(ARCHIVE_FORMAT paxr)
set(COMPRESSION_TYPE BZip2)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("425a68" LIMIT 3 HEX)
