set(OUTPUT_NAME "test.tar.lzma")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=pax --cmake-tar-compression-method=lzma)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("5d0000[08]00[04]" LIMIT 5 HEX)
