enable_language(Swift)

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_package(XCTest REQUIRED)

add_executable(TestedApp MACOSX_BUNDLE dummy_main.swift)

xctest_add_bundle(TestingAppBundle TestedApp dummy_main.swift)

get_target_property(_lib_output_dir TestingAppBundle LIBRARY_OUTPUT_DIRECTORY)

if (NOT DEFINED TEST_EXPECTED_OUTPUT_DIR)
    message(FATAL_ERROR "Testing variable TEST_EXPECTED_OUTPUT_DIR is not set")
endif()

if (NOT _lib_output_dir STREQUAL TEST_EXPECTED_OUTPUT_DIR)
    message(SEND_ERROR "Property LIBRARY_OUTPUT_DIRECTORY is expected to be ${TEST_EXPECTED_OUTPUT_DIR} "
        "but was ${_lib_output_dir}")
endif()
