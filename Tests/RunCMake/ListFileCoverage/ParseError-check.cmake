set(expect_lcov ParseError-lcov.txt)
set(forbid_sf_regex "ParseErrorLib")
include(${RunCMake_SOURCE_DIR}/check-lcov.cmake)
