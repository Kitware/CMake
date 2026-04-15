# Test that file(ARCHIVE_EXTRACT) can extract to a symlinked directory

set(OUTPUT_NAME "test.tar.gz")

set(ARCHIVE_FORMAT gnutar)
set(COMPRESSION_TYPE GZip)
set(DESTINATION_SYMLINK ON)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)
