set(expect_sf_regex "CheckBuildSystem/CMakeLists\\.txt$")
set(forbid_sf_regex "check\\.cmake$" "PreLoad\\.cmake$")
include(${RunCMake_SOURCE_DIR}/check-lcov.cmake)
