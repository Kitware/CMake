set(OUTPUT_NAME "test.7z")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=7zip --cmake-tar-compression-method=ppmd)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

# libarchive 3.8.2 enables a checksum feature; older versions do not.
check_magic("377abcaf271c" LIMIT 6 HEX)
