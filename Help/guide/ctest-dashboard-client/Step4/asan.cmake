set(CTEST_BUILD_NAME "Step4: AddressSanitizer")
site_name(CTEST_SITE)
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/Project")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

set(CTEST_UPDATE_COMMAND "git")
set(CTEST_UPDATE_VERSION_ONLY 1)

set(CTEST_USE_LAUNCHERS 1)
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build-asan")

# Tell CTest that the tests use AddressSanitizer, so that it knows how to
# parse their output.
set(CTEST_MEMORYCHECK_TYPE "AddressSanitizer")

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

ctest_start(Experimental)
ctest_update()

# Build with the -fsanitize=address flag to enable AddressSanitizer.
set(options
  "-DCMAKE_BUILD_TYPE=Debug"
  "-DCMAKE_C_FLAGS=-fsanitize=address"
  )
ctest_configure(OPTIONS "${options}")

ctest_build()

# Run the tests with dynamic analysis instead of calling ctest_test().
ctest_memcheck()

ctest_submit()
