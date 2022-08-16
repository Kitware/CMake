include(CTest)

add_test(NAME cmake_environment COMMAND "${CMAKE_COMMAND}" -E environment)
set_tests_properties(
    cmake_environment
    PROPERTIES
    ENVIRONMENT "CTEST_TEST_VAR=set-via-ENVIRONMENT-property"
    ENVIRONMENT_MODIFICATION "CTEST_TEST_VAR=set:set-via-ENVIRONMENT_MODIFICATION;CTEST_TEST_VAR=reset:"
)
