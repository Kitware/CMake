include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/Metadata-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "1.2.3" "version")
expect_value("${content}" "1.2.0" "compat_version")
expect_value("${content}" "simple" "version_schema")

expect_array("${content}" 1 "default_components")
expect_value("${content}" "foo" "default_components" 0)

expect_array("${content}" 2 "configurations")
expect_value("${content}" "release" "configurations" 0)
expect_value("${content}" "debug" "configurations" 1)
