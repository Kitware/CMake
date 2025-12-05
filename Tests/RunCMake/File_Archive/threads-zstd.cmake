set(OUTPUT_NAME "test.tar.zstd")

set(ARCHIVE_FORMAT pax)
set(COMPRESSION_TYPE Zstd)

include(${CMAKE_CURRENT_LIST_DIR}/threads.cmake)

check_threads("0")
check_threads("1")
check_threads("4")
