include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/DependencyVersionCps-build/")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_array("${content}" 1 "requires" "baz" "components")
expect_value("${content}" "baz" "requires" "baz" "components" 0)
expect_value("${content}" "1.3.5" "requires" "baz" "version")
expect_array("${content}" 1 "requires" "baz" "hints")
expect_value("${content}" "${CMAKE_CURRENT_LIST_DIR}/cps" "requires" "baz" "hints" 0)

string(JSON component GET "${content}" "components" "foo")
expect_array("${component}" 1 "requires")
expect_value("${component}" "baz:baz" "requires" 0)
