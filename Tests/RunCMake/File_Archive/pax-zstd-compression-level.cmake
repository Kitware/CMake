set(OUTPUT_NAME "test.tar.zstd")

set(ARCHIVE_FORMAT pax)
set(COMPRESSION_TYPE Zstd)

include(${CMAKE_CURRENT_LIST_DIR}/compression-level.cmake)

check_compression_level("1")
check_compression_level("5")
check_compression_level("9")
