set(OUTPUT_NAME "test.zip")

set(ARCHIVE_FORMAT zip)

# Combine inclusion and exclusion patterns.  When an entry matches both,
# the exclusion takes precedence.
set(DECOMPRESSION_OPTIONS
  PATTERNS
    "compress_dir/d1/*"    # include contents of d1
    "compress_dir/d_4/*"   # include contents of d_4 ...
  PATTERNS_EXCLUDE
    "d_4"                  # ... but exclude d_4: exclusion wins
)

# Only the included-and-not-excluded entry survives.
set(CUSTOM_CHECK_FILES
  "d1/f1.txt"
)

set(NOT_EXISTING_FILES_CHECK
  "f1.txt"        # never included by PATTERNS
  "d 2/f1.txt"    # never included by PATTERNS
  "d_4/f1.txt"    # included by PATTERNS but excluded: exclusion wins
)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

check_magic("504b0304" LIMIT 4 HEX)
