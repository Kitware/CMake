include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/DependencyVersionCMake-build")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_array("${content}" 1 "requires" "bar" "components")
expect_value("${content}" "bar" "requires" "bar" "components" 0)
expect_value("${content}" "1.3.5" "requires" "bar" "version")
expect_array("${content}" 1 "requires" "bar" "hints")
expect_value("${content}" "${CMAKE_CURRENT_LIST_DIR}/config" "requires" "bar" "hints" 0)

string(JSON component GET "${content}" "components" "foo")
expect_array("${component}" 1 "requires")
expect_value("${component}" "bar:bar" "requires" 0)
