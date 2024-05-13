# This file mimics one containing GoogleTest-related helper functions. It
# includes the GoogleTest module that (in this test) is NOT included in the
# top-level CMake file.

include(GoogleTest)

function(add_gtest_executable TARGET SOURCE)
  add_executable(${TARGET} ${SOURCE})
  xcode_sign_adhoc(${TARGET})
  gtest_discover_tests(
    ${TARGET}
  )
endfunction()
