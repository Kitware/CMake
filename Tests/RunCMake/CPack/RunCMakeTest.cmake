include(RunCMake)
include("${RunCMake_SOURCE_DIR}/CPackTestHelpers.cmake")

# args: TEST_NAME "GENERATORS" RUN_CMAKE_BUILD_STEP
run_cpack_test(MINIMAL "RPM;DEB" false)
