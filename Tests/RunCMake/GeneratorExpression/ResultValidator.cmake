
function (CHECK_VALUE test_msg value expected)
  if (NOT value STREQUAL expected)
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [[${value}]]\nbut expected:\n [[${expected}]]\n")
  endif()
endfunction()
