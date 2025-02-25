include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/TargetTypes-build")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "archive"    "components" "foo-static" "type")
expect_value("${content}" "dylib"      "components" "foo-shared" "type")
expect_value("${content}" "module"     "components" "foo-module" "type")
expect_value("${content}" "interface"  "components" "bar"        "type")
expect_value("${content}" "executable" "components" "test"       "type")
