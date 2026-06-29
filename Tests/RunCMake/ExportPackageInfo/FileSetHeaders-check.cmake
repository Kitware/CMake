include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/FileSetHeaders-build")

file(READ "${out_dir}/cps/foo/foo.cps" content)

string(JSON component GET "${content}" "components" "foo")

expect_array("${component}" 1 "includes")
expect_value("${component}" "${CMAKE_CURRENT_LIST_DIR}/foo" "includes" 0)

expect_array("${component}" 2 "file_sets")

expect_value("${component}" "includes" "file_sets" 0 "type")
expect_value("${component}" "${CMAKE_CURRENT_LIST_DIR}/foo"
             "file_sets" 0 "root")
expect_array("${component}" 2 "file_sets" 0 "files")
expect_value("${component}" "header1.h" "file_sets" 0 "files" 0)
expect_value("${component}" "header2.h" "file_sets" 0 "files" 1)
expect_value("${component}" "no_genex"
             "file_sets" 0 "extensions" "cmake" "name@v1")

expect_value("${component}" "includes" "file_sets" 1 "type")
expect_value("${component}" "${CMAKE_CURRENT_LIST_DIR}/foo"
             "file_sets" 1 "root")
expect_array("${component}" 1 "file_sets" 1 "files")
expect_value("${component}" "header3.h" "file_sets" 1 "files" 0)
expect_value("${component}" "genex"
             "file_sets" 1 "extensions" "cmake" "name@v1")
