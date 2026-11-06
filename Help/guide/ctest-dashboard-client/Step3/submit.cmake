set(CTEST_BUILD_NAME "Step3: coverage")
site_name(CTEST_SITE)
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/../Project")
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

set(CTEST_UPDATE_COMMAND "git")
set(CTEST_UPDATE_VERSION_ONLY 1)

set(CTEST_USE_LAUNCHERS 1)

# Tell CTest to use gcov for coverage analysis.
set(CTEST_COVERAGE_COMMAND "gcov")

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

ctest_start(Experimental)
ctest_update()

# Build with the --coverage flag to enable gcov-style instrumentation.
set(options
  "-DCMAKE_BUILD_TYPE=Debug"
  "-DCMAKE_CXX_FLAGS=--coverage"
  )
ctest_configure(OPTIONS "${options}")

ctest_build()
ctest_test()

# Run gcov to find which lines of code the tests executed.
ctest_coverage()

ctest_submit()
