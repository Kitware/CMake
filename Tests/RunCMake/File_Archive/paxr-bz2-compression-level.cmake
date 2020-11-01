set(OUTPUT_NAME "test.tar.bz2")

set(ARCHIVE_FORMAT paxr)
set(COMPRESSION_TYPE BZip2)

include(${CMAKE_CURRENT_LIST_DIR}/compression-level.cmake)

check_compression_level("1")
check_compression_level("5")
check_compression_level("9")
