set(OUTPUT_NAME "test.tar.xz")

set(COMPRESSION_FLAGS cvJf)
set(COMPRESSION_OPTIONS --cmake-tar-threads=4)

set(DECOMPRESSION_FLAGS xvJf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("fd377a585a00" LIMIT 6 HEX)
