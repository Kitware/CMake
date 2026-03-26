set(OUTPUT_NAME "test.7z")

set(ARCHIVE_FORMAT 7zip)
set(ENCODING OEM)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("377abcaf271c" LIMIT 6 HEX)
