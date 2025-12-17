set(OUTPUT_NAME "test.tar")

set(ARCHIVE_FORMAT gnutar)
set(COMPRESSION_TYPE PPMd)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)
