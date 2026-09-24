include(RunCMake)

list(APPEND RunCMake_TEST_OPTIONS -Wno-experimental)

set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}")

if(SYCL_TEST_CASE STREQUAL "UnsupportedGenerators")
  run_cmake(UnsupportedGenerator-Xcode)
else()
  run_cmake(${SYCL_TEST_CASE})
endif()
