# A generator can run more cmake processes during the try_compile build.
set(expect_files "")
string(REGEX REPLACE "([][+.*?()^$|])" "\\\\\\1" modules_regex
  "${CMAKE_ROOT}/Modules/")
set(expect_sf_regex
  "/CMakeLists\\.txt$"
  "/Configure\\.cmake$"
  "/TryCompileProject/CMakeLists\\.txt$"
  "^${modules_regex}"
  )
# The DEFER command and the deferred set() both count on line 1.
set(expect_lcov_regex "(^|\n)SF:<SRC>/Configure\\.cmake\nFNF:0\nFNH:0\nDA:1,2\n")
include(${RunCMake_SOURCE_DIR}/check-lcov.cmake)
