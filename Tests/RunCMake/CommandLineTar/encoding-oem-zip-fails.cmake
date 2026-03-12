set(OUTPUT_NAME "bad.zip")

set(COMPRESSION_FLAGS cvf)
set(COMPRESSION_OPTIONS --format=zip --cmake-tar-encoding=OEM)
include(${CMAKE_CURRENT_LIST_DIR}/utf-8-paths.cmake)

set(DECOMPRESSION_FLAGS xvf)
set(DECOMPRESSION_OPTIONS --cmake-tar-encoding=OEM)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
