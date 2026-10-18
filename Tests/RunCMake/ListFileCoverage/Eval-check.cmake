set(expect_lcov Eval-lcov.txt)
set(forbid_sf_regex ":EVAL")
include(${RunCMake_SOURCE_DIR}/check-lcov.cmake)
