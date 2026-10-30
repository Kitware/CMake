set(ENV{CTEST_USE_INSTRUMENTATION} "ON")

set(CTEST_BUILD_NAME "Step7: authenticated submissions")
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

set(logs "${CTEST_BINARY_DIRECTORY}/logs.tar.gz")
file(ARCHIVE_CREATE
  OUTPUT "${logs}"
  PATHS "${CTEST_BINARY_DIRECTORY}/Testing/Temporary"
  FORMAT gnutar
  COMPRESSION GZip
  )
ctest_upload(FILES
  "${logs}"
  "${CTEST_SCRIPT_DIRECTORY}/../Step6/cmake.url"
  )

# Submit to a project that requires authentication.  The bearer token is
# read from the CDASH_TOKEN environment variable.
if(NOT DEFINED ENV{CDASH_TOKEN})
  message(FATAL_ERROR "Set the CDASH_TOKEN environment variable")
endif()
ctest_submit(
  SUBMIT_URL "https://my.cdash.org/submit.php?project=gated-sandbox"
  HTTPHEADER "Authorization: Bearer $ENV{CDASH_TOKEN}"
  )
