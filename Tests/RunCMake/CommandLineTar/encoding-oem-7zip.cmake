set(OUTPUT_NAME "test.7z")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=7zip --cmake-tar-encoding=OEM)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

set(DECOMPRESSION_FLAGS xvf)
set(DECOMPRESSION_OPTIONS --cmake-tar-encoding=OEM)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("377abcaf271c" LIMIT 6 HEX)
