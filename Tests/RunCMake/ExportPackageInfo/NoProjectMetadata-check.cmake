include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/NoProjectMetadata-build")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_missing("${content}" "version")
expect_missing("${content}" "compat_version")
expect_missing("${content}" "description")
expect_missing("${content}" "website")
