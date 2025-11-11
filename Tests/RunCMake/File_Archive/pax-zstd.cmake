set(OUTPUT_NAME "test.tar.zstd")

set(ARCHIVE_FORMAT pax)
set(COMPRESSION_TYPE Zstd)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

# libarchive 3.8.2 enables a checksum feature; older versions do not.
check_magic("^28b52ffd0[04]58$" LIMIT 6 HEX)
