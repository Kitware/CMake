include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/Appendix-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "interface" "components" "mammal" "type")
expect_value("${content}" "1.0" "version")

file(READ "${out_dir}/foo-dog.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "interface" "components" "canine" "type")
expect_missing("${content}" "version")

expect_array("${content}" 1 "components" "canine" "requires")
expect_value("${content}" ":mammal" "components" "canine" "requires" 0)
