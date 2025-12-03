set(OUTPUT_NAME "test.tar.bz2")

set(ARCHIVE_FORMAT paxr)
set(COMPRESSION_TYPE BZip2)

include(${CMAKE_CURRENT_LIST_DIR}/threads.cmake)

check_threads("0")
check_threads("1")
check_threads("4")
