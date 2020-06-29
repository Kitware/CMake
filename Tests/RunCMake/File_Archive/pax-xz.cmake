set(OUTPUT_NAME "test.tar.xz")

set(ARCHIVE_FORMAT pax)
set(COMPRESSION_TYPE XZ)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("fd377a585a00" LIMIT 6 HEX)
