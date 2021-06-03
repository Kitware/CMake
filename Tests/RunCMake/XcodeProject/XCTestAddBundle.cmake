enable_language(Swift)
find_package(XCTest REQUIRED)

add_executable(TestedApp MACOSX_BUNDLE EXCLUDE_FROM_ALL foo.swift)

xctest_add_bundle(TestingAppBundle TestedApp foo.swift)

get_target_property(_lib_output_dir TestingAppBundle LIBRARY_OUTPUT_DIRECTORY)

if (NOT DEFINED TEST_EXPECTED_OUTPUT_DIR)
    message(FATAL_ERROR "Testing variable TEST_EXPECTED_OUTPUT_DIR is not set")
endif()

if (NOT _lib_output_dir STREQUAL TEST_EXPECTED_OUTPUT_DIR)
    message(SEND_ERROR "Property LIBRARY_OUTPUT_DIRECTORY is expected to be ${TEST_EXPECTED_OUTPUT_DIR} "
        "but was ${_lib_output_dir}")
endif()
