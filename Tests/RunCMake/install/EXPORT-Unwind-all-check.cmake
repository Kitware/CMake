cmake_policy(SET CMP0074 NEW)
set(Unwind_ROOT "${RunCMake_TEST_BINARY_DIR}/root-all")

find_package(Unwind CONFIG QUIET)

if(Unwind_FOUND)
  set(RunCMake_TEST_FAILED "Unwinding failed, package was reported as found")
endif()
