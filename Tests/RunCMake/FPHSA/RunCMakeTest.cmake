include(RunCMake)

run_cmake(BadFoundVar)
run_cmake(NameMismatch)

# The pseudo module will "find" a package with the given version. Check if the
# version selection code in FPHSA works correctly.

# Find a package with version 0.
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudo_VERSION=0")
run_cmake(any_version_find_0)

# Find a package with more customary version number, without requesting a specific version and in
# the presence of a cache variable VERSION.
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudoNoVersionVar_VERSION=1.2.3.4_SHOULD_BE_IGNORED" "-DVERSION=BAD_VERSION")
run_cmake(any_version_VERSION_cache_variable)

# Find a package with a more customary version number, without requesting a specific version.
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudo_VERSION=1.2.3.4")
run_cmake(any_version)

# test EXACT mode with every subcomponent
run_cmake(exact_1)
run_cmake(exact_1.2)
run_cmake(exact_1.2.3)
run_cmake(exact_1.2.3.4)

# now test every component with an invalid version
set(RunCMake_DEFAULT_stderr ".")
run_cmake(exact_0)
run_cmake(exact_2)
run_cmake(exact_1.1)
run_cmake(exact_1.3)
run_cmake(exact_1.2.2)
run_cmake(exact_1.2.4)
run_cmake(exact_1.2.3.3)
run_cmake(exact_1.2.3.5)
unset(RunCMake_DEFAULT_stderr)

# check if searching for a version 0 works
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudo_VERSION=0")
run_cmake(exact_0_matching)

set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudo_VERSION=")
run_cmake(empty_version)

set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}")
run_cmake(exact_1_no_version_var)

# check custom error message
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DCustomMessage_VERSION=1.2.3.4")
run_cmake(custom_message_1)
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DCONFIG_MODE=TRUE")
run_cmake(custom_message_2)
run_cmake(custom_message_3)

# check handling of components
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DUseComponents_VERSION=1.2.3.4")
run_cmake(required_components)
run_cmake(required_and_optional_components)
run_cmake(all_optional_components)
list(APPEND RunCMake_TEST_OPTIONS "-DUseComponents_REQUIRE_VARS=TRUE")
run_cmake(required_components_with_vars)

# check handling of version range
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudo_VERSION=2.3.4.5")
run_cmake(range_ignored)
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" "-DPseudoRange_VERSION=2.0")
run_cmake(range_no-range)
run_cmake(range_1-3)
run_cmake(range_1-2-include)
run_cmake(range_1-2-exclude)
run_cmake(range_3-4)
