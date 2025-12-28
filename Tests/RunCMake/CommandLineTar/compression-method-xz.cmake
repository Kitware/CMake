set(OUTPUT_NAME "test.tar.xz")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --cmake-tar-compression-method=xz)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("fd377a585a00" LIMIT 6 HEX)
