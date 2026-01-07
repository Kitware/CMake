set(OUTPUT_NAME "test.tar.gz")

set(COMPRESSION_FLAGS -cvf)
set(COMPRESSION_OPTIONS --format=gnutar --cmake-tar-compression-method=deflate)

set(DECOMPRESSION_FLAGS -xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("1f8b" LIMIT 2 HEX)
