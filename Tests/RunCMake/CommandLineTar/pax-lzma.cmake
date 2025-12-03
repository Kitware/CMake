set(OUTPUT_NAME "test.tar.xz")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=pax --lzma)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("5d00008000" LIMIT 5 HEX)
