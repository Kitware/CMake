set(OUTPUT_NAME "test.zip")

# No need to set CMP0213 - WARN is default

set(ARCHIVE_FORMAT zip)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
