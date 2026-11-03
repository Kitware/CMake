set(expect_sf_regex "/Callbacks\\.cmake$")
set(expect_lcov_regex
  "\nDA:15,1\n"
  "\nDA:16,1\n"
  "\nDA:17,1\n"
  "\nFNDA:3,keep\n"
  "\nFNDA:[1-9][0-9]*,cmp\n"
  "\nFNDA:3,twice\n"
  )
include(${RunCMake_SOURCE_DIR}/check-lcov.cmake)
