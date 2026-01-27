enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(test_workdir test_workdir.cpp)
xcode_sign_adhoc(test_workdir)

set(workdir "${CMAKE_CURRENT_BINARY_DIR}/with spaces")
file(WRITE ${workdir}/test_list_output.txt [=[
WorkDirWithSpaces.
  test1
  test2
]=])
file(WRITE ${workdir}/test_list_output.json [=[
{
    "tests": 2,
    "name": "AllTests",
    "testsuites": [
        {
            "name": "WorkDirWithSpaces",
            "tests": 2,
            "testsuite": [
                { "name": "test1", "file": "file.cpp", "line": 42 },
                { "name": "test2", "file": "file.cpp", "line": 43 }
            ]
        }
    ]
}
]=])
file(WRITE ${workdir}/test_output.txt [=[
Some output text for the test.
]=])

gtest_add_tests(
  TARGET test_workdir
  WORKING_DIRECTORY "${workdir}"
)
gtest_discover_tests(
  test_workdir
  TEST_SUFFIX _discovered
  WORKING_DIRECTORY "${workdir}"
)
