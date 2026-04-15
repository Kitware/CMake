# Test that cmake -E tar can extract to a symlinked directory

set(OUTPUT_NAME "test.tar.gz")

set(COMPRESSION_FLAGS -cvzf)
set(COMPRESSION_OPTIONS --format=gnutar)

set(DECOMPRESSION_FLAGS -xvzf)

set(DESTINATION_SYMLINK ON)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)
