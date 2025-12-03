set(OUTPUT_NAME "test.tar.gz")

set(ARCHIVE_FORMAT gnutar)
set(COMPRESSION_TYPE GZip)

include(${CMAKE_CURRENT_LIST_DIR}/threads.cmake)

check_threads("-1")
