file(READ "${RunCMake_TEST_BINARY_DIR}/tp.txt" actual)
string(STRIP "${actual}" actual)
# flatMap: libA -> /a/inc (1 dir), libB -> /b/inc1;/b/inc2 (2 dirs) => 3 items.
if(NOT actual STREQUAL "/a/inc;/b/inc1;/b/inc2")
  set(RunCMake_TEST_FAILED
    "unexpected APPLY target-property output: [${actual}]")
endif()
