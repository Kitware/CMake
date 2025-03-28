project(${RunCMake_TEST} LANGUAGES NONE)

message(STATUS
  "From subdir1:\n"
  "  ${RunCMake_TEST}_SOURCE_DIR = ${${RunCMake_TEST}_SOURCE_DIR}\n"
  "  ${RunCMake_TEST}_BINARY_DIR = ${${RunCMake_TEST}_BINARY_DIR}\n"
  "  ${RunCMake_TEST}_IS_TOP_LEVEL = ${${RunCMake_TEST}_IS_TOP_LEVEL}"
)
