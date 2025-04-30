include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

file(READ "${RunCMake_BINARY_DIR}/install/test/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "interface" "components" "foo" "type")
