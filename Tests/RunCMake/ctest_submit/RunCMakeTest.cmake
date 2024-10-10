include(RunCTest)

# Default case parameters.
set(CASE_DROP_METHOD "http")
set(CASE_DROP_SITE "badhostname.invalid")
set(CASE_CTEST_SUBMIT_ARGS "")
set(CASE_TEST_PREFIX_CODE "")

# Do not use any proxy for lookup of an invalid site.
# DNS failure by proxy looks different than DNS failure without proxy.
set(ENV{no_proxy} "$ENV{no_proxy},badhostname.invalid")

function(run_ctest_submit CASE_NAME)
  set(CASE_CTEST_SUBMIT_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

function(run_ctest_submit_debug CASE_NAME)
  set(CASE_CTEST_SUBMIT_ARGS "${ARGN}")
  run_ctest(${CASE_NAME} "--debug")
endfunction()

#-----------------------------------------------------------------------------
# Test bad argument combinations.
run_ctest_submit(BadArg bad-arg)
run_ctest_submit(BadPARTS PARTS bad-part)
run_ctest_submit(BadFILES FILES bad-file)
run_ctest_submit(RepeatRETURN_VALUE RETURN_VALUE res RETURN_VALUE res)
run_ctest_submit(PARTSCDashUpload PARTS Configure CDASH_UPLOAD)
run_ctest_submit(PARTSCDashUploadType PARTS Configure CDASH_UPLOAD_TYPE)
run_ctest_submit(PARTSDone PARTS Done)
run_ctest_submit(CDashUploadPARTS CDASH_UPLOAD bad-upload PARTS)
run_ctest_submit(CDashUploadFILES CDASH_UPLOAD bad-upload FILES)
run_ctest_submit(CDashUploadNone CDASH_UPLOAD)
run_ctest_submit(CDashUploadMissingFile CDASH_UPLOAD bad-upload)
run_ctest_submit(CDashUploadRetry CDASH_UPLOAD ${CMAKE_CURRENT_LIST_FILE} CDASH_UPLOAD_TYPE foo RETRY_COUNT 2 RETRY_DELAY 1 INTERNAL_TEST_CHECKSUM)
run_ctest_submit(CDashSubmitQuiet QUIET)
run_ctest_submit_debug(CDashSubmitVerbose BUILD_ID my_build_id)
run_ctest_submit_debug(FILESNoBuildId FILES ${CMAKE_CURRENT_LIST_FILE})
run_ctest_submit_debug(CDashSubmitHeaders HTTPHEADER "Authorization: Bearer asdf")
run_ctest_submit_debug(CDashUploadHeaders CDASH_UPLOAD ${CMAKE_CURRENT_LIST_FILE} CDASH_UPLOAD_TYPE foo HTTPHEADER "Authorization: Bearer asdf")

function(run_ctest_CDashUploadFTP)
  set(CASE_DROP_METHOD ftp)
  run_ctest_submit(CDashUploadFTP CDASH_UPLOAD ${CMAKE_CURRENT_LIST_FILE})
endfunction()
run_ctest_CDashUploadFTP()

#-----------------------------------------------------------------------------
# Test failed drops by various protocols

function(run_ctest_submit_FailDrop CASE_DROP_METHOD)
  run_ctest(FailDrop-${CASE_DROP_METHOD})
endfunction()

run_ctest_submit_FailDrop(http)
run_ctest_submit_FailDrop(https)
block()
  set(CASE_DROP_METHOD "https")
  set(CASE_TEST_PREFIX_CODE "set(CTEST_TLS_VERSION 1.1)")
  run_ctest(FailDrop-TLSVersion-1.1 -VV)
  set(CASE_TEST_PREFIX_CODE "set(CMAKE_TLS_VERSION 1.1)") # Test fallback to CMake variable.
  run_ctest(FailDrop-TLSVersion-1.1-cmake -VV)
  set(ENV{CMAKE_TLS_VERSION} 1.1) # Test fallback to env variable.
  set(CASE_TEST_PREFIX_CODE "")
  run_ctest(FailDrop-TLSVersion-1.1-env -VV)
  unset(ENV{CMAKE_TLS_VERSION})
  set(CASE_TEST_PREFIX_CODE "set(CTEST_TLS_VERIFY ON)")
  run_ctest(FailDrop-TLSVerify-ON -VV)
  set(CASE_TEST_PREFIX_CODE "set(CMAKE_TLS_VERIFY ON)") # Test fallback to CMake variable.
  run_ctest(FailDrop-TLSVerify-ON-cmake -VV)
  set(ENV{CMAKE_TLS_VERIFY} ON) # Test fallback to env variable.
  set(CASE_TEST_PREFIX_CODE "")
  run_ctest(FailDrop-TLSVerify-ON-env -VV)
  unset(ENV{CMAKE_TLS_VERIFY})
  set(CASE_TEST_PREFIX_CODE "set(CTEST_TLS_VERIFY OFF)")
  run_ctest(FailDrop-TLSVerify-OFF -VV)
  set(CASE_TEST_PREFIX_CODE "set(CMAKE_TLS_VERIFY OFF)") # Test fallback to CMake variable.
  run_ctest(FailDrop-TLSVerify-OFF-cmake -VV)
  set(ENV{CMAKE_TLS_VERIFY} OFF) # Test fallback to env variable.
  set(CASE_TEST_PREFIX_CODE "")
  run_ctest(FailDrop-TLSVerify-OFF-env -VV)
  unset(ENV{CMAKE_TLS_VERIFY})
endblock()
