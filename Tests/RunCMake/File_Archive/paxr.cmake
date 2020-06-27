set(OUTPUT_NAME "test.tar")

set(ARCHIVE_FORMAT paxr)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("7573746172003030" OFFSET 257 LIMIT 8 HEX)
