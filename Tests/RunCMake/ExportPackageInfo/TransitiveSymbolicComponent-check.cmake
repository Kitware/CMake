include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/TransitiveSymbolicComponent-build")


file(READ "${out_dir}/bar.cps" content)
expect_value("${content}" "bar" "name")
expect_array("${content}" 1 "requires" "Symbolic" "components")
expect_value("${content}" "test" "requires" "Symbolic" "components" 0)
expect_value("${content}" "1.0" "requires" "Symbolic" "version")
expect_array("${content}" 1 "requires" "Symbolic" "hints")
expect_value("${content}" "${CMAKE_CURRENT_LIST_DIR}/cps" "requires" "Symbolic" "hints" 0)

string(JSON component GET "${content}" "components" "bar")
expect_array("${component}" 1 "requires")
expect_value("${component}" "Symbolic:test" "requires" 0)
