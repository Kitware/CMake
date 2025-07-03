include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/EmptyConfig-build")

file(READ "${out_dir}/foo.cps" content)

expect_object("${content}" "components" "foo" "configurations" "noconfig")
