include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/FileSetHeaders-build")

file(READ "${out_dir}/foo.cps" content)

string(JSON component GET "${content}" "components" "foo")

expect_array("${component}" 1 "includes")
expect_value("${component}" "${CMAKE_CURRENT_LIST_DIR}/foo" "includes" 0)
