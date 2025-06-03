include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/ProjectMetadata-build")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "1.2.3" "version")
expect_value("${content}" "1.1.0" "compat_version")

file(READ "${out_dir}/test1.cps" content)
expect_value("${content}" "test1" "name")
expect_value("${content}" "1.2.3" "version")
expect_value("${content}" "1.1.0" "compat_version")

file(READ "${out_dir}/test2.cps" content)
expect_value("${content}" "test2" "name")
expect_value("${content}" "1.4.7" "version")
expect_missing("${content}" "compat_version")
