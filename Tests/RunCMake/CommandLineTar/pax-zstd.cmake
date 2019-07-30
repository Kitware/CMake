set(OUTPUT_NAME "test.tar.zstd")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=pax --zstd)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("28b52ffd0058" LIMIT 6 HEX)
