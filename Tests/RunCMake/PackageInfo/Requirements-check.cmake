include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/Requirements-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "interface" "components" "libb" "type")

file(READ "${out_dir}/bar.cps" content)
expect_value("${content}" "bar" "name")
expect_array("${content}"      1 "requires"  "foo" "components")
expect_value("${content}" "libb" "requires"  "foo" "components" 0)
expect_array("${content}"      1 "requires" "test" "components")
expect_value("${content}" "liba" "requires" "test" "components" 0)
expect_value("${content}" "interface" "components" "libc" "type")
expect_value("${content}" "interface" "components" "libd" "type")

string(JSON component GET "${content}" "components" "libd")
expect_array("${component}" 3 "requires")
expect_value("${component}" "test:liba" "requires" 0)
expect_value("${component}"  "foo:libb" "requires" 1)
expect_value("${component}"     ":libc" "requires" 2)
