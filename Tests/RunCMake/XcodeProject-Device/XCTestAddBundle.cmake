enable_language(Swift)

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_package(XCTest REQUIRED)

add_executable(TestedApp MACOSX_BUNDLE dummy_main.swift)

xctest_add_bundle(TestingAppBundle TestedApp dummy_main.swift)

macro(add_test NAME name COMMAND xctest arg)
  set(actual_arg "${arg}" PARENT_SCOPE)
endmacro()

xctest_add_test(TestedApp.TestingAppBundle TestingAppBundle)

if(NOT DEFINED TEST_EXPECTED_OUTPUT_DIR)
  message(FATAL_ERROR "Testing variable TEST_EXPECTED_OUTPUT_DIR is not set")
endif()
set(expect_arg "${TEST_EXPECTED_OUTPUT_DIR}/$<TARGET_BUNDLE_DIR_NAME:TestingAppBundle>")
if(NOT "${actual_arg}" STREQUAL "${expect_arg}")
  message(FATAL_ERROR "xctest argument expected to be:\n"
    "  ${expect_arg}\n"
    "but was:\n"
    "  ${actual_arg}\n"
  )
endif()
