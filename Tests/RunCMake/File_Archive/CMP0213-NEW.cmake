set(OUTPUT_NAME "test.zip")

cmake_policy(SET CMP0213 NEW)

set(ARCHIVE_FORMAT zip)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
