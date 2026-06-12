set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT zip)

# Extract the whole archive except entries matching PATTERNS_EXCLUDE.
set(DECOMPRESSION_OPTIONS
  PATTERNS_EXCLUDE
    "compress_dir/d 2/*"   # exclude everything under a directory
    "d-4"                  # exclude by unanchored name (dir + contents)
    "no_such_entry"        # matches nothing: must not be an error
)

# Everything that was not excluded must still be extracted.
set(CUSTOM_CHECK_FILES
  "f1.txt"
  "d1/f1.txt"
  "d + 3/f1.txt"
  "d_4/f1.txt"                   # underscore, not the excluded "d-4"
  "My Special Directory/f1.txt"
)

# The excluded entries must not be extracted.
set(NOT_EXISTING_FILES_CHECK
  "d 2/f1.txt"
  "d-4/f1.txt"
)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
