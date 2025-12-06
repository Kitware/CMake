set(OUTPUT_NAME "test.7z")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --cmake-tar-compression-level=19 --format=7zip --zstd)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("377abcaf271c" LIMIT 6 HEX)
