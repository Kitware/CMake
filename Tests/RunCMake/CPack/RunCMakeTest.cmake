cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

include(RunCMake)
include("${RunCMake_SOURCE_DIR}/CPackTestHelpers.cmake")

# args: TEST_NAME "GENERATORS" RUN_CMAKE_BUILD_STEP
run_cpack_test(MINIMAL "RPM;DEB;TGZ" false)
run_cpack_test(PARTIALLY_RELOCATABLE_WARNING "RPM" false)
run_cpack_test(DEB_EXTRA "DEB" false)
run_cpack_test(DEPENDENCIES "RPM;DEB" true)
run_cpack_test(EMPTY_DIR "RPM;DEB;TGZ" true)
run_cpack_test(COMPONENTS_EMPTY_DIR "RPM;DEB;TGZ" true)
