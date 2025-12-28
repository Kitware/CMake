set(OUTPUT_NAME "test.tar.bz2")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=paxr --cmake-tar-compression-method=bzip2)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("425a68" LIMIT 3 HEX)
