set(OUTPUT_NAME "bad.zip")

set(ARCHIVE_FORMAT zip)
set(ENCODING OEM)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
