include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/Requirements-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "interface" "components" "libb" "type")

file(READ "${out_dir}/bar.cps" content)
expect_value("${content}" "bar" "name")
expect_null("${content}" "requires" "foo")
expect_null("${content}" "requires" "test")
expect_value("${content}" "interface" "components" "libc" "type")
expect_value("${content}" "interface" "components" "libd" "type")

string(JSON component GET "${content}" "components" "libd")
expect_array("${component}" 3 "requires")
expect_value("${component}" "test:liba" "requires" 0)
expect_value("${component}"  "foo:libb" "requires" 1)
expect_value("${component}"     ":libc" "requires" 2)
