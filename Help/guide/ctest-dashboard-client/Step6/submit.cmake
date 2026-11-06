set(ENV{CTEST_USE_INSTRUMENTATION} "ON")

set(CTEST_BUILD_NAME "Step6: uploads")
site_name(CTEST_SITE)
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/../Project")
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

set(CTEST_UPDATE_COMMAND "git")
set(CTEST_UPDATE_VERSION_ONLY 1)

set(CTEST_USE_LAUNCHERS 1)
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

# Bundle the logs from this run into an archive.
set(logs "${CTEST_BINARY_DIRECTORY}/logs.tar.gz")
file(ARCHIVE_CREATE
  OUTPUT "${logs}"
  PATHS "${CTEST_BINARY_DIRECTORY}/Testing/Temporary"
  FORMAT gnutar
  COMPRESSION GZip
  )

# Upload extra files to be associated with this build on CDash.
ctest_upload(FILES
  "${logs}"
  "${CTEST_SCRIPT_DIRECTORY}/cmake.url"
  )

ctest_submit()
