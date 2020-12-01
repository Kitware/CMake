set(OUTPUT_NAME "test.7z")

set(ARCHIVE_FORMAT 7zip)

include(${CMAKE_CURRENT_LIST_DIR}/compression-level.cmake)

check_compression_level("1")
