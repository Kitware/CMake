set(OUTPUT_NAME "test.tar")

set(ARCHIVE_FORMAT gnutar)
set(ENCODING UTF-8)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("7573746172202000" OFFSET 257 LIMIT 8 HEX)
