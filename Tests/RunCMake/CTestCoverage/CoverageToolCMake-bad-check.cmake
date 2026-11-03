set(tree "${RunCMake_TEST_BINARY_DIR}")
# The script's own file is merged.  The scratch folder stays for inspection.
foreach(path IN ITEMS "bad.cmake-coverage.info" "bad_cmake-coverage/bad.info")
  if(NOT EXISTS "${tree}/${path}")
    string(APPEND RunCMake_TEST_FAILED "Missing ${tree}/${path}\n")
  endif()
endforeach()
