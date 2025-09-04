set(fbuild_bff "${RunCMake_TEST_BINARY_DIR}/fbuild.bff")

if(NOT EXISTS "${fbuild_bff}")
  set(RunCMake_TEST_FAILED "Generator output file is missing:\n ${fbuild_bff}")
  return()
endif()
file(READ "${fbuild_bff}" fbuild_bff)

if(NOT fbuild_bff MATCHES ${REGEX_TO_MATCH})
    set(RunCMake_TEST_FAILED "Regex '${REGEX_TO_MATCH}' not found in the generated file ${RunCMake_TEST_BINARY_DIR}/fbuild.bff")
endif()
