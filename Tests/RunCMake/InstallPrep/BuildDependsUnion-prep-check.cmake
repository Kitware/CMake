# Under a multi-configuration generator, preparing the Debug configuration
# also builds what the entry evaluated for Release names.  A single
# configuration generator evaluates the entries for its one configuration.
set(debug_built "${RunCMake_TEST_BINARY_DIR}/union-debug-built.txt")
set(release_built "${RunCMake_TEST_BINARY_DIR}/union-release-built.txt")
if(NOT EXISTS "${debug_built}")
  set(RunCMake_TEST_FAILED
    "Preparation did not build the Debug dependency union_debug.")
elseif(RunCMake_GENERATOR_IS_MULTI_CONFIG AND NOT EXISTS "${release_built}")
  set(RunCMake_TEST_FAILED "Preparation for Debug did not build the \
Release dependency, so the rule does not depend on the union of the \
entries evaluated for each configuration.")
elseif(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG AND EXISTS "${release_built}")
  set(RunCMake_TEST_FAILED
    "Preparation built the Release dependency, but only Debug is configured.")
endif()
