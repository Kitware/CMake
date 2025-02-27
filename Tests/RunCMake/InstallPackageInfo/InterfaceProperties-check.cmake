include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/InterfaceProperties-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")

string(JSON component GET "${content}" "components" "foo")

expect_value("${component}" "interface" "type")
expect_array("${component}" 1 "includes")
expect_value("${component}" "@prefix@/include/foo" "includes" 0)
expect_array("${component}" 1 "compile_features")
expect_value("${component}" "c++23" "compile_features" 0)
expect_array("${component}" 1 "compile_flags")
expect_value("${component}" "-ffast-math" "compile_flags" 0)
expect_null("${component}" "compile_definitions" "*" "FOO")
expect_value("${component}" "BAR" "compile_definitions" "*" "BAR")
expect_array("${component}" 1 "link_directories")
expect_value("${component}" "/opt/foo/lib" "link_directories" 0)
expect_array("${component}" 1 "link_flags")
expect_value("${component}" "--needed" "link_flags" 0)
expect_array("${component}" 1 "link_libraries")
expect_value("${component}" "/usr/lib/libm.so" "link_libraries" 0)
