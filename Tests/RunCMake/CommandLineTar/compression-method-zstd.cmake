set(OUTPUT_NAME "test.tar.zstd")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --cmake-tar-compression-method=zstd)

set(DECOMPRESSION_FLAGS xvf)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

# libarchive 3.8.2 enables a checksum feature; older versions do not.
check_magic("^28b52ffd0[04][56]8$" LIMIT 6 HEX)
