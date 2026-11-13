# Report the CMake code of CMake itself, such as `Modules/`, and leave out
# the CMake code of the tests, of the guides, and of the CI files.  CI
# also downloads third-party code into `.gitlab/`.
list(APPEND CTEST_CUSTOM_COVERAGE_EXCLUDE
  "/Tests/"
  "/Help/guide/"
  "/\\.gitlab/"
  )
