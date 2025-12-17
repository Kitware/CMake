set(OUTPUT_NAME "test.7z")

set(ARCHIVE_FORMAT 7zip)
set(COMPRESSION_TYPE LZMA)

include(${CMAKE_CURRENT_LIST_DIR}/compression-level.cmake)

check_compression_level("1")
check_compression_level("5")
check_compression_level("9")
