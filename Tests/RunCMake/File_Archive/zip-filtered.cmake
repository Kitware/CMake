set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT zip)

set(DECOMPRESSION_OPTIONS
  PATTERNS
    compress_dir/f1.txt # Decompress only file
    compress_*/d?       # and whole directory (has only one match)
)

set(CUSTOM_CHECK_FILES
  "f1.txt"
  "d1/f1.txt"
)

# This files shouldn't exists
set(NOT_EXISTING_FILES_CHECK
  "d 2/f1.txt"
  "d + 3/f1.txt"
  "d_4/f1.txt"
  "d-4/f1.txt"
)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
