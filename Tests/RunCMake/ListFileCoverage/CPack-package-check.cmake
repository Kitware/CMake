# cpack records its configuration file and the install scripts it runs.
set(expect_sf_regex
  "/CPackConfig\\.cmake$"
  "/cmake_install\\.cmake$"
  )
include(${RunCMake_SOURCE_DIR}/check-lcov.cmake)
