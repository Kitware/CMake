set(unitybuild_0 "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Unity/unity_0_cxx.cxx")

file(STRINGS ${unitybuild_0} src)

foreach(expectedRegex IN ITEMS "SRC_f\\.cxx" "BLD_s1\\.cpp")
  if(NOT "${src}" MATCHES "${expectedRegex}")
    set(RunCMake_TEST_FAILED "Generated unity file doesn't have a match for expected unity ID regex ${expectedRegex}")
    return()
  endif()
endforeach()
