include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/InstallSymbolicComponent-build/CMakeFiles/Export/5058f1af8388633f609cadb75a75dc9d")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_value("${content}" "symbolic" "components" "foo" "type")
