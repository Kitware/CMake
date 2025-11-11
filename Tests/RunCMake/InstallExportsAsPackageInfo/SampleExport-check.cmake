include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/SampleExport-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/farm.cps" content)
expect_value("${content}" "farm" "name")
expect_value("${content}" "interface" "components" "cow" "type")
expect_value("${content}" "1.2.3" "version")
expect_value("${content}" "1.1.0" "compat_version")
expect_value("${content}" "simple" "version_schema")
expect_value("${content}" "Apache-2.0" "license")
expect_value("${content}" "BSD-3-Clause" "default_license")
expect_array("${content}" 2 "configurations")
expect_value("${content}" "Small" "configurations" 0)
expect_value("${content}" "Large" "configurations" 1)

file(READ "${out_dir}/farm-extra.cps" content)
expect_value("${content}" "farm" "name")
expect_value("${content}" "interface" "components" "pig" "type")
