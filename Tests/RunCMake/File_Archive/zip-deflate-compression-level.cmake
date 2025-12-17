set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT zip)
set(COMPRESSION_TYPE Deflate)

include(${CMAKE_CURRENT_LIST_DIR}/compression-level.cmake)

check_compression_level("1")
check_compression_level("5")
check_compression_level("9")
