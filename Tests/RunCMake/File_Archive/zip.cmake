set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT zip)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
