include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/ExportSymbolicComponent-build")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "symbolic" "components" "foo" "type")
