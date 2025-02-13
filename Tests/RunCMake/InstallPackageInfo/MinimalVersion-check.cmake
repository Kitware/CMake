include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/MinimalVersion-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo1.cps" content)
expect_value("${content}" "foo1" "name")
expect_value("${content}" "1.0" "version")
expect_missing("${content}" "compat_version")
expect_missing("${content}" "version_schema")

file(READ "${out_dir}/foo2.cps" content)
expect_value("${content}" "foo2" "name")
expect_value("${content}" "1.5" "version")
expect_value("${content}" "1.0" "compat_version")
expect_missing("${content}" "version_schema")

file(READ "${out_dir}/foo3.cps" content)
expect_value("${content}" "foo3" "name")
expect_value("${content}" "1.0" "version")
expect_missing("${content}" "compat_version")
expect_value("${content}" "simple" "version_schema")
