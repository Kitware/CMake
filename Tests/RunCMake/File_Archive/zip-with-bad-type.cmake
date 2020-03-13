set(OUTPUT_NAME "test.zip")

set(COMPRESSION_FORMAT zip)
set(COMPRESSION_TYPE BZip2)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)
