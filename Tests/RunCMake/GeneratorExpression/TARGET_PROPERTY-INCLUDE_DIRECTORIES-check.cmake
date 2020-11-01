file(READ "${RunCMake_TEST_BINARY_DIR}/out.txt" content)

unset(RunCMake_TEST_FAILED)

if (NOT content MATCHES "(INCLUDES1:${RunCMake_TEST_SOURCE_DIR}/include)")
  string(APPEND RunCMake_TEST_FAILED "wrong content for INCLUDES1: \"${CMAKE_MATCH_1}\"\n")
endif()

if (NOT content MATCHES "(INCLUDES2:><)")
  string(APPEND RunCMake_TEST_FAILED "wrong content for INCLUDES2: \"${CMAKE_MATCH_1}\"\n")
endif()
if (NOT content MATCHES "(INCLUDES3:><)")
  string(APPEND RunCMake_TEST_FAILED "wrong content for INCLUDES3: \"${CMAKE_MATCH_1}\"\n")
endif()
if (NOT content MATCHES "(CUSTOM:>;;<)")
  string(APPEND RunCMake_TEST_FAILED "wrong content for CUSTOM: \"${CMAKE_MATCH_1}\"\n")
endif()
