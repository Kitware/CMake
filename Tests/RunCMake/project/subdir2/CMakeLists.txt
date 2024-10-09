message(STATUS
  "From subdir2:\n"
  "  ${RunCMake_TEST}_SOURCE_DIR = ${${RunCMake_TEST}_SOURCE_DIR}\n"
  "  ${RunCMake_TEST}_BINARY_DIR = ${${RunCMake_TEST}_BINARY_DIR}\n"
  "  ${RunCMake_TEST}_IS_TOP_LEVEL = ${${RunCMake_TEST}_IS_TOP_LEVEL}"
)

project(sub2proj LANGUAGES NONE)

message(STATUS
  "  sub2proj_SOURCE_DIR = ${sub2proj_SOURCE_DIR}\n"
  "  sub2proj_BINARY_DIR = ${sub2proj_BINARY_DIR}"
)
