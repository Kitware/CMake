set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT 7zip)
set(COMPRESSION_TYPE XZ)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)
